#include "mesh.h"
#include "draw.h"

Mesh::Mesh() {
	// allocate memory
	posAttrib = new Attribute<vec3>();
	normAttrib = new Attribute<vec3>();
	uvAttrib = new Attribute<vec2>();

	// for gpu skinning
	weightsAttrib = new Attribute<vec4>();
	influencesAttrib = new Attribute<ivec4>();

	indexBuffer = new IndexBuffer();

	morphTargetsAtlas = new DataTexture();
	morphTargetsCount = new int();
}

// copy constructor
Mesh::Mesh(const Mesh& m) {
	posAttrib = new Attribute<vec3>();
	normAttrib = new Attribute<vec3>();
	uvAttrib = new Attribute<vec2>();

	// for gpu skinning
	weightsAttrib = new Attribute<vec4>();
	influencesAttrib = new Attribute<ivec4>();

	indexBuffer = new IndexBuffer();

	morphTargetsAtlas = new DataTexture();
	morphTargetsCount = new int();
	*this = m;
}

// assign mesh
Mesh& Mesh::operator=(const Mesh& other) {
	if (this == &other) {
		return *this;
	}
	// copy out the CPU values
	positions = other.positions;
	normals = other.normals;
	texCoords = other.texCoords;
	weights = other.weights;
	influences = other.influences;
	indices = other.indices;

	morphTargets = other.morphTargets;
	material = other.material;
	name = other.name;
	// upload the attribute data to the GPU
	updateOpenGLBuffers();
	return *this;
}

Mesh::~Mesh() {
	// free memory
	delete posAttrib;
	delete normAttrib;
	delete uvAttrib;

	delete weightsAttrib;
	delete influencesAttrib;

	delete indexBuffer;

}

// getters
std::vector<vec3>& Mesh::getPositions() {
	return positions;
}

std::vector<vec3>& Mesh::getNormals() {
	return normals;
}

std::vector<vec2>& Mesh::getTexCoords() {
	return texCoords;
}

std::vector<vec4>& Mesh::getWeights() {
	return weights;
}

std::vector<ivec4>& Mesh::getInfluences() {
	return influences;
}

std::vector<unsigned int>& Mesh::getIndices() {
	return indices;
}

std::vector<MorphTarget>& Mesh::getMorphTargets() {
	return morphTargets;
}

DataTexture* Mesh::getMorphTargetsAtlas() {
	return morphTargetsAtlas;
}

int Mesh::getMorphTargetsCount() {
	return *morphTargetsCount;
}

Material& Mesh::getMaterial() {
	return material;
}

// setters
void Mesh::setPositions(std::vector<vec3> pos) {
	positions = pos;
}

void Mesh::setMaterial(Material mat) {
	material = mat;
}

// Define the size of the texture atlas
const int ATLAS_WIDTH = 800;
const int ATLAS_HEIGHT = 800;

// Function to encode morph target data into the texture atlas
void Mesh::encodeMorphTargets() {
	int offsetX = 0;
	int offsetY = 0;

	// Resize the texture assuming it will have enough space to encode all the morph targets
	morphTargetsAtlas->Resize(ATLAS_WIDTH);

	// [CA] To do: Encode each morph ta rget into the texture data
    for (int i = 0; i < morphTargets.size(); ++i) {

        std::vector<vec3> vOffsets = morphTargets[i].vertexOffsets;
        int numVertices = vOffsets.size();

        // Encode the vertex offsets of the current morph target
        for (int j = 0; j < numVertices; ++j) 
        {
            // x and y positions in texture atlas
            // int x = (offsetX + j) % ATLAS_WIDTH;
            // int y = offsetY + (offsetX + j) / ATLAS_WIDTH;

            // Encode the morph target data into the texture atlas
            morphTargetsAtlas->SetTexel(i, j, vOffsets[j]); //(x, y, vOffsets[j])
        }

        // update offsest
        // offsetY = offsetY + (numVertices + offsetX) / ATLAS_WIDTH;
        // offsetX = (offsetX + numVertices) % ATLAS_WIDTH;
    }

	// Loads the texture data into the GPU
	morphTargetsAtlas->UploadTextureDataToGPU();
}

// CPU skinning using matrices
void Mesh::CPUSkin(Skeleton& skeleton, Pose& pose) {

	unsigned int numVerts = positions.size();
	if (numVerts == 0) { return; }

	// Make sure the skinned vectors have enough storage space
	skinnedPositions.resize(numVerts);
	skinnedNormals.resize(numVerts);

	// get the global matrices from the current pose and set it to the corresponent array
	poseMatrices = pose.getGlobalMatrices();
	// get the inverse bind pose stored in the skeleton
	std::vector<mat4> invBindPoseMat = skeleton.getInvBindPose();

	for (unsigned int i = 0; i < numVerts; i++) //i = vertex
	{
		ivec4& joints = influences[i];
		vec4& weight = weights[i];

		// TODO: Compute the skin matrix. For each vertex get the scaled skin matrix:
		// 
		// encontrar para cada joint de un vertex su matrix (4 matrices cada vertex)
		// Get the matrix multiplying the transform of the joint pose (current/animated pose) by the inverse bind pose first (to get the vertex in the joint space)

		mat4 jointPoseTransform_x = poseMatrices[joints.x];
		mat4 jointPoseTransform_y = poseMatrices[joints.y];
		mat4 jointPoseTransform_z = poseMatrices[joints.z];
		mat4 jointPoseTransform_w = poseMatrices[joints.w];

		mat4 invBindPose_x = invBindPoseMat[joints.x];
		mat4 invBindPose_y = invBindPoseMat[joints.y];
		mat4 invBindPose_z = invBindPoseMat[joints.z];
		mat4 invBindPose_w = invBindPoseMat[joints.w];

		mat4 jointSkinMatrix_x = jointPoseTransform_x * invBindPose_x;
		mat4 jointSkinMatrix_y = jointPoseTransform_y * invBindPose_y;
		mat4 jointSkinMatrix_z = jointPoseTransform_z * invBindPose_z;
		mat4 jointSkinMatrix_w = jointPoseTransform_w * invBindPose_w;

		// Scale the resulting matrices by their corresponding weight to get the final skin matrix

		jointSkinMatrix_x = jointSkinMatrix_x * weight.x;
		jointSkinMatrix_y = jointSkinMatrix_y * weight.y;
		jointSkinMatrix_z = jointSkinMatrix_z * weight.z;
		jointSkinMatrix_w = jointSkinMatrix_w * weight.w;

		mat4 finalSkinMatrix = jointSkinMatrix_x + jointSkinMatrix_y + jointSkinMatrix_z + jointSkinMatrix_w;

		// Get the skinned position of the vertex (object local space)
		skinnedPositions[i] = transformPoint(finalSkinMatrix, positions[i]);

		// Get the skinned normal vector of the vertex (object local space)
		skinnedNormals[i] = transformVector(mat4ToTransform(finalSkinMatrix), normals[i]);
	}

	// update the GPU position and normal attributes with the skinned positions and normals of all vertices
	posAttrib->Set(skinnedPositions);
	normAttrib->Set(skinnedNormals);
}


// upload attribute data to the GPU 
void Mesh::updateOpenGLBuffers() {
	if (positions.size() > 0) {
		posAttrib->Set(positions);
	}
	if (normals.size() > 0) {
		normAttrib->Set(normals);
	}
	if (texCoords.size() > 0) {
		uvAttrib->Set(texCoords);
	}
	if (weights.size() > 0) {
		weightsAttrib->Set(weights);
	}
	if (influences.size() > 0) {
		influencesAttrib->Set(influences);
	}
	if (indices.size() > 0) {
		indexBuffer->Set(indices);
	}
	if (morphTargets.size() > 0) {
		encodeMorphTargets();
		morphTargetsAtlas->UploadTextureDataToGPU();
	}
	
}

// bind the attributes to the specified slots
void Mesh::bind(int position, int normal, int uv, int weight, int influence) {
	if (position >= 0) {
		posAttrib->BindTo(position);
	}
	if (normal >= 0) {
		normAttrib->BindTo(normal);
	}
	if (uv >= 0) {
		uvAttrib->BindTo(uv);
	}
	if (weight >= 0) {
		weightsAttrib->BindTo(weight);
	}
	if (influence >= 0) {
		influencesAttrib->BindTo(influence);
	}
}

// drawing calls to the GPU
void Mesh::draw() {
	if (indices.size() > 0) {
		::Draw(*indexBuffer, DrawMode::Triangles);
	}
	else {
		::Draw(positions.size(), DrawMode::Triangles);
	}
}

void Mesh::drawInstanced(unsigned int numInstances) {
	if (indices.size() > 0) {
		::DrawInstanced(*indexBuffer, DrawMode::Triangles, numInstances);
	}
	else {
		::DrawInstanced(positions.size(), DrawMode::Triangles, numInstances);
	}
}

// bind the attributes of the specified slots
void Mesh::unBind(int position, int normal, int uv, int weight, int influence) {
	if (position >= 0) {
		posAttrib->UnBindFrom(position);
	}
	if (normal >= 0) {
		normAttrib->UnBindFrom(normal);
	}
	if (uv >= 0) {
		uvAttrib->UnBindFrom(uv);
	}
	if (weight >= 0) {
		weightsAttrib->UnBindFrom(weight);
	}
	if (influence >= 0) {
		influencesAttrib->UnBindFrom(influence);
	}
}


