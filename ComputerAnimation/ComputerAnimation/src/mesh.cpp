#include "mesh.h"
#include "shading/draw.h"

Mesh::Mesh() {
	// allocate memory
	posAttrib = new Attribute<vec3>();
	normAttrib = new Attribute<vec3>();
	uvAttrib = new Attribute<vec2>();
}

// copy constructor
Mesh::Mesh(const Mesh& m) {
	posAttrib = new Attribute<vec3>();
	normAttrib = new Attribute<vec3>();
	uvAttrib = new Attribute<vec2>();

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

	for (unsigned int i = 0; i < numVerts; i++) {
		ivec4& joints = influences[i];
		vec4& weight = weights[i];

		// TODO: Compute the skin matrix:
		// For each vertex get the scaled skin matrix:
		//		1. Get the matrix multiplying the transform of the joint pose (current/animated pose) by the inverse bind pose first (to get the vertex in the joint space)
		// Scale the resulting matrices by their corresponding weight to get the final skin matrix
		// Get the skinned position of the vertex (object local space)
		// Get the skinned normal vector of the vertex (object local space)

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
}

// bind the attributes to the specified slots
void Mesh::bind(int position, int normal, int uv, int weight, int influcence) {
	if (position >= 0) {
		posAttrib->BindTo(position);
	}
	if (normal >= 0) {
		normAttrib->BindTo(normal);
	}
	if (uv >= 0) {
		uvAttrib->BindTo(uv);
	}
}

// drawing calls to the GPU
void Mesh::draw() {

	::Draw(positions.size(), DrawMode::Triangles);

}

void Mesh::drawInstanced(unsigned int numInstances) {

	::DrawInstanced(positions.size(), DrawMode::Triangles, numInstances);

}
// bind the attributes of the specified slots
void Mesh::unBind(int position, int normal, int uv, int weight, int influcence) {
	if (position >= 0) {
		posAttrib->UnBindFrom(position);
	}
	if (normal >= 0) {
		normAttrib->UnBindFrom(normal);
	}
	if (uv >= 0) {
		uvAttrib->UnBindFrom(uv);
	}
}