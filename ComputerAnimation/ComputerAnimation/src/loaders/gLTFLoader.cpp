#include "gLTFLoader.h"
#include <iostream>
#include <algorithm>
#include "../math/transform.h"
#include "../skeleton.h"

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
		for (unsigned int j = 0; j < numJoints; j++) 
		{
			//get the 16 values of the inverse bind matrix and put them into a mat4
			float* matrix = &(invBindAccessor[j * 16]);
			mat4 invBindMatrix = mat4(matrix);

			Transform bindTransform;
			//TODO: invert the matrix (to get the transfrom matrix of the joint) and convert it into a transform
			bindTransform = mat4ToTransform(inverse(invBindMatrix));

			//set the transform into the array of transforms of the joints in the bind pose (world bind pose)
			cgltf_node* jointNode = skin->joints[j];
			int jointIndex = GLTFHelpers::getNodeIndex(jointNode, data->nodes, numBones);
			worldBindPose[jointIndex] = bindTransform;
		}
	}

	//convert the world bind pose to a regular bind pose
	Pose bindPose = restPose;

	for (unsigned int i = 0; i < numBones; i++) 
	{
		Transform current = worldBindPose[i];
		int parentId = bindPose.getParent(i);

		//if the bone has a parent, bring it into parent space (local space)
		if (parentId >= 0) 
		{
			Transform parent = worldBindPose[parentId];
			//TODO: invert the parent transformation and combine the transformations
			Transform invertparent = inverse(parent);
			current = combine(parent, current);
		}
		//TODO: set the local transform of the joint to the bind pose
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
	//TODO: create an skeleton loading the bind pose, the rest pose and the joint names from the gltf data 
	Pose restPose = loadRestPose(data);
	Pose bindPose = loadBindPose(data);
	std::vector<std::string> jointNames = loadJointNames(data);

	// Construct and return the Skeleton
	Skeleton skeleton;
	skeleton.set(restPose, bindPose, jointNames);
	return skeleton;
}

std::vector<Mesh> loadMeshes(const cgltf_data* data) {
	std::vector<Mesh> result;
	cgltf_node* nodes = data->nodes;
	unsigned int nodeCount = data->nodes_count;

	for (unsigned int i = 0; i < nodeCount; ++i) {
		cgltf_node* node = &nodes[i];
		// Only process nodes that have both a mesh and a skin
		if (node->mesh == 0 || node->skin == 0) {
			continue;
		}

		int numPrims = node->mesh->primitives_count;
		for (int j = 0; j < numPrims; ++j) {

			//create a mesh for each primitive
			result.push_back(Mesh());
			Mesh& mesh = result[result.size() - 1];

			cgltf_primitive* primitive = &node->mesh->primitives[j];

			// Loop through all the attributes in the primitive and populate the mesh data
			unsigned int ac = primitive->attributes_count;
			for (unsigned int k = 0; k < ac; ++k) {
				cgltf_attribute* attribute = &primitive->attributes[k];
				GLTFHelpers::meshFromAttribute(mesh, *attribute, node->skin, nodes, nodeCount);

				// if the primitive has indices, the index buffer of the mesh needs to be filled out
				if (primitive->indices != 0) {
					int ic = primitive->indices->count;
					std::vector<unsigned int>& indices = mesh.getIndices();
					indices.resize(ic);

					for (unsigned int k = 0; k < ic; ++k) {
						indices[k] = cgltf_accessor_read_index(primitive->indices, k);
					}
				}
				//for render the mesh
				mesh.updateOpenGLBuffers();
			}
		}
	}
	//return the resulting vector of meshes
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
	std::vector<vec2>& texCoords = outMesh.getUVs();
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