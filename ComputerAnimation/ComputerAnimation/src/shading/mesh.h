#pragma once
#include <vector>
#include "../math/vec2.h"
#include "../math/vec3.h"
#include "../math/vec4.h"
#include "../math/mat4.h"
#include "../shading/attribute.h"
#include "../animation/skeleton.h"
#include "../animation/pose.h"
#include "indexBuffer.h"

class Mesh
{
protected:
	std::vector<vec3> positions;
	std::vector<vec3> normals;
	std::vector<vec2> uvs;
	// each vertex can be influenced at most by 4 bones (vec4)
	std::vector<vec4> weights;
	std::vector<ivec4> influences; // joint ids of each vertex
	std::vector<unsigned int> indices; // vertex indices

	// for CPU skinning
	std::vector<vec3> skinnedPositions;
	std::vector<vec3> skinnedNormals;
	std::vector<mat4> poseMatrices; // global matrices of the pose joints

	// for render in the GPU
	Attribute<vec3>* posAttrib;
	Attribute<vec3>* normAttrib;
	Attribute<vec2>* uvAttrib;
	Attribute<vec4>* weightAttrib;
	Attribute<ivec4>* influenceAttrib;

	IndexBuffer* indexBuffer;


public:
	Mesh();
	Mesh(const Mesh&);
	Mesh& operator=(const Mesh&);
	~Mesh();

	// getters
	std::vector<vec3>& getPositions();
	std::vector<vec3>& getNormals();
	std::vector<vec2>& getUVs();
	std::vector<vec4>& getWeights();
	std::vector<ivec4>& getInfluences();
	std::vector<unsigned int>& getIndices();

	// setter
	void setPositions(std::vector<vec3> positions);
	// applies CPU mesh-skinning
	void CPUSkin(Skeleton& skeleton, Pose& pose);
	// syncs the vectors holding data to the GPU
	void updateOpenGLBuffers();

	void bind(int position, int normal, int uv, int weight, int influence); // the parameters are the slots 
	void draw();
	void drawInstanced(unsigned int numInstances);
	void unBind(int position, int normal, int uv, int weight, int influence); // the parameters are the slots 
};
