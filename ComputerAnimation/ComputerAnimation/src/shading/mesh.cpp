#include "mesh.h"
#include "../shading/draw.h"
#include <iostream>

Mesh::Mesh() {
	// allocate memory
	posAttrib = new Attribute<vec3>();
	normAttrib = new Attribute<vec3>();
	uvAttrib = new Attribute<vec2>();
	weightAttrib = new Attribute<vec4>();
	influenceAttrib = new Attribute<ivec4>();

	indexBuffer = new IndexBuffer();
}

// copy constructor
Mesh::Mesh(const Mesh& m) {
	posAttrib = new Attribute<vec3>();
	normAttrib = new Attribute<vec3>();
	uvAttrib = new Attribute<vec2>();
	weightAttrib = new Attribute<vec4>();
	influenceAttrib = new Attribute<ivec4>();

	indexBuffer = new IndexBuffer();

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
	uvs = other.uvs;
	weights = other.weights;
	influences = other.influences;
	indices = other.indices;

	// upload the attribute data to the GPU
	updateOpenGLBuffers();
	return *this;
}

Mesh::~Mesh() {
	// free memory
	delete posAttrib;
	delete normAttrib;
	delete uvAttrib;
	delete weightAttrib;
	delete influenceAttrib;
	delete indexBuffer;
}

// getters
std::vector<vec3>& Mesh::getPositions() {
	return positions;
}
std::vector<vec3>& Mesh::getNormals() {
	return normals;
}
std::vector<vec2>& Mesh::getUVs() {
	return uvs;
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

// setters
void Mesh::setPositions(std::vector<vec3> pos) {
	positions = pos;
}




// CPU skinning using matrices
//void Mesh::CPUSkin(Skeleton& skeleton, Pose& pose) {
//	unsigned int numVerts = positions.size();
//	if (numVerts == 0) { return; }
//
//	// Make sure the skinned vectors have enough storage space
//	skinnedPositions.resize(numVerts);
//	skinnedNormals.resize(numVerts);
//	Pose& bindPose = skeleton.getBindPose();
//	for (unsigned int i = 0; i < numVerts; i++) {
//		ivec4& joint = influences[i];
//		vec4& weight = weights[i];
//		Transform skin0 = combine(pose[joint.x], inverse(bindPose[joint.x]));
//		vec3 p0 = transformPoint(skin0, positions[i]);
//		vec3 n0 = transformVector(skin0, normals[i]);
//		// repeat 3 more times
//		Transform skin1 = combine(pose[joint.y], inverse(bindPose[joint.y]));
//		vec3 p1 = transformPoint(skin1, positions[i]);
//		vec3 n1 = transformVector(skin1, normals[i]);
//		Transform skin2 = combine(pose[joint.z], inverse(bindPose[joint.z]));
//		vec3 p2 = transformPoint(skin2, positions[i]);
//		vec3 n2 = transformVector(skin2, normals[i]);
//		Transform skin3 = combine(pose[joint.w], inverse(bindPose[joint.w]));
//		vec3 p3 = transformPoint(skin3, positions[i]);
//		vec3 n3 = transformVector(skin3, normals[i]);
//		skinnedPositions[i] = p0 * weight.x +
//			p1 * weight.y +
//			p2 * weight.z +
//			p3 * weight.w;
//		skinnedNormals[i] = n0 * weight.x +
//			n1 * weight.y +
//			n2 * weight.z +
//			n3 * weight.w;
//	}
//	// update the GPU position and normal attributes with the skinned positions and normals of all vertices
//	posAttrib->Set(skinnedPositions);
//	normAttrib->Set(skinnedNormals);
//}

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
	if (uvs.size() > 0) {
		uvAttrib->Set(uvs);
	}
	if (weights.size() > 0) {
		weightAttrib->Set(weights);
	}
	if (influences.size() > 0) {
		influenceAttrib->Set(influences);
	}
	if (indices.size() > 0) {
		indexBuffer->Set(indices);
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
		weightAttrib->BindTo(weight);
	}
	if (influence >= 0) {
		influenceAttrib->BindTo(influence);
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
		weightAttrib->UnBindFrom(weight);
	}
	if (influence >= 0) {
		influenceAttrib->UnBindFrom(influence);
	}
}