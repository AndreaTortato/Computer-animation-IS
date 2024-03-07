#include "gLTFLoader.h"
#include <iostream>
#include <algorithm>
#include "../math/transform.h"
#include "../animation/skeleton.h"
#include <vector>
#include "../external/stb_image.h"
#include <cassert>
//takes a path and returns a cgltf_data pointer
cgltf_data* loadGLTFFile(const char* path) {
	cgltf_options options;
	memset(&options, 0, sizeof(cgltf_options));
	cgltf_data* data = NULL;
	cgltf_result result = cgltf_parse_file(&options, path, &data);
	if (result != cgltf_result_success) {
		std::cout << "Could not load: " << path << "\n";
		return 0;
	}
	result = cgltf_load_buffers(&options, data, path);
	if (result != cgltf_result_success) {
		cgltf_free(data);
		std::cout << "Could not load: " << path << "\n";
		return 0;
	}
	result = cgltf_validate(data);
	if (result != cgltf_result_success) {
		cgltf_free(data);
		std::cout << "Invalid file: " << path << "\n";
		return 0;
	}
	return data;
}

void freeGLTFFile(cgltf_data* data) {
	if (data == 0) {
		std::cout << "WARNING: Can't free null data\n";
	}
	else {
		cgltf_free(data);
	}
}

Pose loadRestPose(const cgltf_data* data) {
	unsigned int numBones = data->nodes_count;
	Pose restPose(numBones);

	for (unsigned int i = 0; i < numBones; i++) {
		cgltf_node* node = &(data->nodes[i]);
		//get local transform
		Transform transform = GLTFHelpers::getLocalTransform(data->nodes[i]);
		//assign the local transform to the pose's joint
		restPose.setLocalTransform(i, transform);
		//get parent node index
		int parentId = GLTFHelpers::getNodeIndex(node->parent, data->nodes, numBones);
		//set the parennt to the pose joint
		restPose.setParent(i, parentId);
	}
	return restPose;
}

Pose loadBindPose(const cgltf_data* data) {
	Pose restPose = loadRestPose(data);

	unsigned int numBones = restPose.size();
	std::vector<Transform> worldBindPose(numBones);
	// initialize the array of the world transforms with the transforms of the rest pose
	for (unsigned int i = 0; i < numBones; i++) {
		worldBindPose[i] = restPose.getGlobalTransform(i);
	}

	//get number of skins (skinned meshes)
	unsigned int numSkins = data->skins_count;
	for (unsigned int i = 0; i < numSkins; i++) {
		cgltf_skin* skin = &(data->skins[i]);
		// read the inverse bind pose matrix into a large vector of float values
		std::vector<float> invBindAccessor;
		GLTFHelpers::getScalarValues(invBindAccessor, 16, *skin->inverse_bind_matrices);

		//for each joint in the skin, get the inverse bind pose matrix
		unsigned int numJoints = skin->joints_count;
		for (unsigned int j = 0; j < numJoints; j++) {
			//get the 16 values of the inverse bind matrix and put them into a mat4
			float* matrix = &(invBindAccessor[j * 16]);
			mat4 invBindMatrix = mat4(matrix);
			// invert, convert to transform
			mat4 bindMatrix = inverse(invBindMatrix);
			Transform bindTransform = mat4ToTransform(bindMatrix);
			//set the transform into the array of transforms of the joints in the bind pose (world bind pose)
			cgltf_node* jointNode = skin->joints[j];
			int jointIndex = GLTFHelpers::getNodeIndex(jointNode, data->nodes, numBones);
			worldBindPose[jointIndex] = bindTransform;
		}
	}

	//convert the world bind pose to a regular bind pose
	Pose bindPose = restPose;
	for (unsigned int i = 0; i < numBones; ++i) {
		Transform current = worldBindPose[i];
		int p = bindPose.getParent(i);
		if (p >= 0) { // Bring into parent space
			Transform parent = worldBindPose[p];
			current = combine(inverse(parent), current);
		}
		bindPose.setLocalTransform(i, current);
	}
	return bindPose;
}

//loads the names of every joint in the same order that the joints for the rest pose were loaded
std::vector<std::string> loadJointNames(const cgltf_data* data) {
	unsigned int numBones = (unsigned int)data->nodes_count;
	std::vector<std::string> result(numBones, "Not Set");
	for (unsigned int i = 0; i < numBones; ++i) {
		cgltf_node* node = &(data->nodes[i]);
		if (node->name == 0) {
			result[i] = "EMPTY NODE";
		}
		else {
			result[i] = node->name;
		}
	}
	return result; 
}

Skeleton loadSkeleton(const cgltf_data* data) {
	return Skeleton(
		loadRestPose(data),
		loadBindPose(data),
		loadJointNames(data)
	);
}

std::vector<Mesh> loadMeshes(const cgltf_data* data) {
	std::vector<Mesh> result;
	std::vector<std::vector<MorphTarget> > morphTargets;
	
	cgltf_node* nodes = data->nodes;
	unsigned int nodeCount = data->nodes_count;

	for (unsigned int i = 0; i < nodeCount; ++i) {
		cgltf_node* node = &nodes[i];
		// Only process nodes that have both a mesh and a skin
		if (node->mesh == 0 ) {
			continue;
		}

		int numPrims = node->mesh->primitives_count;
		for (int j = 0; j < numPrims; ++j) {

			// Create a mesh for each primitive
			
			result.push_back(Mesh());
			Mesh &mesh = result[result.size()-1];
			
			cgltf_primitive* primitive = &node->mesh->primitives[j];
			const char* name = node->mesh->name;
			std::string str(name);
			mesh.name = str;
			// Loop through all the attributes in the primitive and populate the mesh data
			unsigned int ac = primitive->attributes_count;
			for (unsigned int k = 0; k < ac; ++k) {
				cgltf_attribute* attribute = &primitive->attributes[k];
				GLTFHelpers::meshFromAttribute(mesh, *attribute, node->skin, nodes, nodeCount);

				// If the primitive has indices, the index buffer of the mesh needs to be filled out
				if (primitive->indices != 0) {
					int ic = primitive->indices->count;
					std::vector<unsigned int>& indices = mesh.getIndices();
					indices.resize(ic);

					for (unsigned int k = 0; k < ic; ++k) {
						indices[k] = cgltf_accessor_read_index(primitive->indices, k);
					}
				}				
			}

			// Assign the material information to the mesh
			if (primitive->material) {
				GLTFHelpers::materialFromPimitive(mesh, *primitive);
			}
			
			// Loop through all the morph targets in the primitive and populate the mesh data
			if (primitive->targets_count) {
				GLTFHelpers::morphTargetsFromPimitive(mesh, *primitive);

				std::vector<char*> names;
				for (unsigned int n = 0; n < node->mesh->target_names_count; n++) {
					mesh.getMorphTargets()[n].name = node->mesh->target_names[n];
					names.push_back(node->mesh->target_names[n]);

				}
				mesh.morphTargetNames = names;
			}

			// For render the mesh
			mesh.updateOpenGLBuffers();
		}
	}

	// Return the resulting vector of meshes
	return result;
}


std::vector<Clip> loadAnimationClips(cgltf_data* data) {
	unsigned int nuclips = data->animations_count;
	unsigned int numNodes = data->nodes_count;

	std::vector<Clip> result;
	result.resize(nuclips);

	for (unsigned int i = 0; i < nuclips; ++i) {
		// set the name of the clip
		result[i].setName(data->animations[i].name);

		// each channel of a glTF file is an animation track
		unsigned int numChannels = data->animations[i].channels_count;
		for (unsigned int j = 0; j < numChannels; ++j) {
			cgltf_animation_channel& channel = data->animations[i].channels[j];
			cgltf_node* target = channel.target_node;

			// find the index of the node that the current channel affects
			int nodeId = GLTFHelpers::getNodeIndex(target, data->nodes, numNodes);
			if (channel.target_path == cgltf_animation_path_type_translation) {
				VectorTrack& track = result[i][nodeId].getPositionTrack();
				// convert the track into an animation track
				GLTFHelpers::trackFromChannel<vec3, 3>(track, channel);
			}
			else if (channel.target_path == cgltf_animation_path_type_scale) {
				VectorTrack& track = result[i][nodeId].getScaleTrack();
				// convert the track into an animation track
				GLTFHelpers::trackFromChannel<vec3, 3>(track, channel);
			}
			else if (channel.target_path == cgltf_animation_path_type_rotation) {
				QuaternionTrack& track = result[i][nodeId].getRotationTrack();
				// convert the track into an animation track
				GLTFHelpers::trackFromChannel<quat, 4>(track, channel);
			}
		} // End num channels loop
		result[i].recalculateDuration();
	} // End num clips loop

	return result;
}

//Gets the local transform of cgltf_node
Transform GLTFHelpers::getLocalTransform(cgltf_node& n) {
	Transform result;

	//if the node stores its transformation as a matrix, use the mat4ToTransform decomposition function;
	if (n.has_matrix) {
		mat4 mat(&n.matrix[0]);
		result = mat4ToTransform(mat);
	}
	if (n.has_translation) {
		result.position = vec3(n.translation[0], n.translation[1], n.translation[2]);
	}
	if (n.has_rotation) {
		result.rotation = quat(n.rotation[0], n.rotation[1], n.rotation[2], n.rotation[3]);
	}

	if (n.has_scale) {
		result.scale = vec3(n.scale[0], n.scale[1], n.scale[2]);
	}
	return result;
}

//Gets the node index given all the nodes
int GLTFHelpers::getNodeIndex(cgltf_node* node, cgltf_node* allNodes, unsigned int numNodes) {
	if (node == 0) {
		return -1;
	}
	for (unsigned int i = 0; i < numNodes; ++i) {
		if (node == &allNodes[i]) {
			return (int)i;
		}
	}
	return -1;
}

//Reads the floating-point values of a gltf accessor and put them into a vector of floats
void GLTFHelpers::getScalarValues(std::vector<float>& out, unsigned int compCount, const cgltf_accessor& inAccessor) {
	out.resize(inAccessor.count * compCount);
	for (cgltf_size i = 0; i < inAccessor.count; ++i) {
		cgltf_accessor_read_float(&inAccessor, i, &out[i * compCount], compCount);
	}
}

/*
It takes a mesh and a cgltf_attribute function, along with some additional data required for parsing.
The attribute contains one of our mesh components, such as the position, normal, UV coordinate, weights, or influences.
This attribute provides the appropriate mesh data
*/
void GLTFHelpers::meshFromAttribute(Mesh& outMesh, cgltf_attribute& attribute, cgltf_skin* skin, cgltf_node* nodes, unsigned int nodeCount) {
	cgltf_attribute_type attribType = attribute.type;
	cgltf_accessor& accessor = *attribute.data;

	//get how many attributes the current component has
	unsigned int componentCount = 0;
	if (accessor.type == cgltf_type_vec2) {
		componentCount = 2;
	}
	else if (accessor.type == cgltf_type_vec3) {
		componentCount = 3;
	}
	else if (accessor.type == cgltf_type_vec4) {
		componentCount = 4;
	}

	//Parse the data
	std::vector<float> values;
	getScalarValues(values, componentCount, accessor);
	unsigned int acessorCount = accessor.count;

	//Create references to the position, normal, texture coordinate, influences, and weights vectors of the mesh
	std::vector<vec3>& positions = outMesh.getPositions();
	std::vector<vec3>& normals = outMesh.getNormals();
	std::vector<vec2>& texCoords = outMesh.getTexCoords();
	std::vector<ivec4>& influences = outMesh.getInfluences();
	std::vector<vec4>& weights = outMesh.getWeights();

	//loop through all the values in the current accessor and assign them to the appropriate vector based on the accessor type
	for (unsigned int i = 0; i < acessorCount; ++i) {
		int index = i * componentCount;
		switch (attribType)
		{
		case cgltf_attribute_type_position:
			positions.push_back(vec3(values[index + 0], values[index + 1], values[index + 2]));
			break;

		case cgltf_attribute_type_normal:
		{
			vec3 normal = vec3(values[index + 0], values[index + 1], values[index + 2]);
			//if the normal is not normalized, normalize it
			if (lenSq(normal) < 0.000001f) {
				normal = vec3(0, 1, 0);
			}
			normals.push_back(normalized(normal));
		}
		break;

		case cgltf_attribute_type_texcoord:
			texCoords.push_back(vec2(values[index + 0], values[index + 1]));
			break;

		case cgltf_attribute_type_weights:
			weights.push_back(vec4(values[index + 0], values[index + 1], values[index + 2], values[index + 3]));
			break;

		case cgltf_attribute_type_joints:
		{
			// Joints are stored as floating-point numbers.Convert them into integers: 
			// These indices are skin relative. This function has no information about the skin that is being parsed.
			// Add +0.5f to round, since we can't read integers
			ivec4 joints(
				(int)(values[index + 0] + 0.5f),
				(int)(values[index + 1] + 0.5f),
				(int)(values[index + 2] + 0.5f),
				(int)(values[index + 3] + 0.5f)
			);

			// convert the joint indices so that they go from being relative to the joints array to being relative to the skeleton hierarchy
			joints.x = getNodeIndex(skin->joints[joints.x], nodes, nodeCount);
			joints.y = getNodeIndex(skin->joints[joints.y], nodes, nodeCount);
			joints.z = getNodeIndex(skin->joints[joints.z], nodes, nodeCount);
			joints.w = getNodeIndex(skin->joints[joints.w], nodes, nodeCount);

			//Make sure that even the invalid nodes have a value of 0 (any negative joint indices will break the skinning implementation)
			joints.x = std::max(0, joints.x);
			joints.y = std::max(0, joints.y);
			joints.z = std::max(0, joints.z);
			joints.w = std::max(0, joints.w);

			influences.push_back(joints);
		}
		break;
		}
	}
}

void GLTFHelpers::morphTargetsFromPimitive(Mesh& outMesh, cgltf_primitive& primitive) {
	std::vector<MorphTarget> &morphTargets = outMesh.getMorphTargets();
	morphTargets.resize(primitive.targets_count);

	// Loop through all the targets in the primitive and populate the mesh data
	for (unsigned int k = 0; k < primitive.targets_count; ++k) {
		cgltf_morph_target* morphTarget = &primitive.targets[k];
	
		MorphTarget& mt = morphTargets[k];

		for (unsigned int t = 0; t < morphTarget->attributes_count; t++) {

			cgltf_attribute_type attribType = morphTarget->attributes[t].type;
			cgltf_accessor& accessor = *morphTarget->attributes[t].data;

			// Get how many attributes the current component has
			unsigned int componentCount = 0;
			if (accessor.type == cgltf_type_vec2) {
				componentCount = 2;
			}
			else if (accessor.type == cgltf_type_vec3) {
				componentCount = 3;
			}
			else if (accessor.type == cgltf_type_vec4) {
				componentCount = 4;
			}

			// Parse the data
			std::vector<float> values;
			getScalarValues(values, componentCount, accessor);
			unsigned int acessorCount = accessor.count;


			// Loop through all the values in the current accessor and assign them to the appropriate vector based on the accessor type
			for (unsigned int i = 0; i < acessorCount; ++i) {
				int index = i * componentCount;
				switch (attribType)
				{
				case cgltf_attribute_type_position:
					mt.vertexOffsets.push_back(vec3(values[index + 0], values[index + 1], values[index + 2]));
					break;

				case cgltf_attribute_type_normal:
				{
					vec3 normal = vec3(values[index + 0], values[index + 1], values[index + 2]);
					// If the normal is not normalized, normalize it
					if (lenSq(normal) < 0.000001f) {
						normal = vec3(0, 1, 0);
					}
					mt.normals.push_back(normalized(normal));
				}
				break;

				}
			}
		}
	}

}

void GLTFHelpers::materialFromPimitive(Mesh& outMesh, cgltf_primitive& primitive) {
	Material& material = outMesh.getMaterial();
	if (primitive.material->has_pbr_metallic_roughness) {

		material.colorTexture = new Texture();

		cgltf_pbr_metallic_roughness pbr_material = primitive.material->pbr_metallic_roughness;
		cgltf_texture* tex = pbr_material.base_color_texture.texture;
		if (tex != NULL) {

			cgltf_buffer_view* b = tex->image->buffer_view;
			int width, height, numComponents;
			const unsigned char* data = b->offset + (unsigned char*)b->buffer->data;
			int byteCount = b->size;
			if (!stbi_info_from_memory(data, byteCount, &width, &height, &numComponents)) {
				assert(false);
			}
			unsigned char* imageData = stbi_load_from_memory(data, byteCount, &width, &height, &numComponents, 4);

			material.colorTexture->Load(imageData, width, height, numComponents);
			stbi_image_free(imageData);
		}
		float* colorFactor = pbr_material.base_color_factor;
		material.baseColor = vec4(*colorFactor, *colorFactor, *colorFactor, 1.f);
		material.metallic = pbr_material.metallic_factor;
		material.roughness = pbr_material.roughness_factor;

		material.metallicTexture = new Texture();
		cgltf_texture* metallicTexture = primitive.material->pbr_metallic_roughness.metallic_roughness_texture.texture;
		if (metallicTexture != NULL) {

			cgltf_buffer_view* b = metallicTexture->image->buffer_view;
			int width, height, numComponents;
			const unsigned char* data = b->offset + (unsigned char*)b->buffer->data;
			int byteCount = b->size;
			if (!stbi_info_from_memory(data, byteCount, &width, &height, &numComponents)) {
				assert(false);
			}
			unsigned char* imageData = stbi_load_from_memory(data, byteCount, &width, &height, &numComponents, 4);
			material.metallicTexture->Load(imageData, width, height, numComponents);
			stbi_image_free(imageData);
		}

	}
	material.normalMap = new Texture();
	cgltf_texture* normalTex = primitive.material->normal_texture.texture;
	if (normalTex != NULL) {

		cgltf_buffer_view* b = normalTex->image->buffer_view;
		int width, height, numComponents;
		const unsigned char* data = b->offset + (unsigned char*)b->buffer->data;
		int byteCount = b->size;
		if (!stbi_info_from_memory(data, byteCount, &width, &height, &numComponents)) {
			assert(false);
		}
		unsigned char* imageData = stbi_load_from_memory(data, byteCount, &width, &height, &numComponents, 4);
		material.normalMap->Load(imageData, width, height, numComponents);
		stbi_image_free(imageData);
	}
	if (primitive.material->has_specular) {
		material.specular = primitive.material->specular.specular_factor;
	}


	material.double_side = primitive.material->double_sided;
	material.alpha_cutoff = primitive.material->alpha_cutoff;
	switch (primitive.material->alpha_mode)
	{
	case cgltf_alpha_mode_blend:
		material.alpha_mode = alphaMode::ALPHA_BLEND;
		break;
	case cgltf_alpha_mode_opaque:
		material.alpha_mode = alphaMode::ALPHA_OPAQUE;
		break;
	case cgltf_alpha_mode_mask:
		material.alpha_mode = alphaMode::ALPHA_MASK;
		break;

	default:
		material.alpha_mode = alphaMode::ALPHA_OPAQUE;
		break;
	}
}


// converts a glTF animation channel into a VectorTrack or a QuaternionTrack
template<typename T, int N>
void GLTFHelpers::trackFromChannel(Track<T, N>& result, const cgltf_animation_channel& channel) {
	cgltf_animation_sampler& sampler = *channel.sampler;
	Interpolation interpolation = Interpolation::Constant;

	// make sure the Interpolation type of the track matches the cgltf_interpolation_type type of the sampler
	if (sampler.interpolation == cgltf_interpolation_type_linear) {
		interpolation = Interpolation::Linear;
	}
	else if (sampler.interpolation == cgltf_interpolation_type_cubic_spline) {
		interpolation = Interpolation::Cubic;
	}
	bool isSamplerCubic = interpolation == Interpolation::Cubic;
	result.setInterpolation(interpolation);

	// convert sampler input and output accessors into linear arrays of floating-point numbers
	std::vector<float> time; // times
	getScalarValues(time, 1, *sampler.input);

	std::vector<float> val; // values
	getScalarValues(val, N, *sampler.output);

	unsigned int numFrames = sampler.input->count;
	unsigned int compCount = val.size() / time.size(); // components (vec3 or quat) per frame
	// resize the track to have enough room to store all the frames
	result.resize(numFrames);

	// parse the time and value arrays into frame structures
	for (unsigned int i = 0; i < numFrames; ++i) {
		int baseIndex = i * compCount;
		Frame<N>& frame = result[i];
		// offset used to deal with cubic tracks since the input and output tangents are as large as the number of components
		int offset = 0;

		frame.time = time[i];

		// read input tangent (only if the sample is cubic)
		for (int comp = 0; comp < N; ++comp) {
			frame.in[comp] = isSamplerCubic ?
				val[baseIndex + offset++] : 0.0f;
		}
		// read the value
		for (int comp = 0; comp < N; ++comp) {
			frame.value[comp] = val[baseIndex +
				offset++];
		}
		// read the output tangent (only if the sample is cubic)
		for (int comp = 0; comp < N; ++comp) {
			frame.out[comp] = isSamplerCubic ?
				val[baseIndex + offset++] : 0.0f;
		}
	}
}