#pragma once
#include <vector>
#include "../math/vec2.h"
#include "../math/vec3.h"
#include "../math/vec4.h"
#include "../math/mat4.h"
#include "attribute.h"
#include "indexBuffer.h"
#include "../animation/skeleton.h"
#include "../animation/pose.h"
#include "texture.h"

struct MorphTarget {
	std::vector<vec3> vertexOffsets;
	std::vector<vec3> normals;
	char* name;
};

enum alphaMode { ALPHA_OPAQUE = 0, ALPHA_BLEND, ALPHA_MASK };
struct Material {
	vec4 baseColor = vec4(1);
	float metallic = 0.04;
	float roughness = 0.8;
	float specular = 0;
	Texture* colorTexture = NULL;
	Texture* emissiveTexture = NULL;
	Texture* normalMap = NULL;
	Texture* occlusionMap = NULL;
	Texture* metallicTexture = NULL;
	bool double_side = false;
	float alpha_cutoff = 0;
	int alpha_mode = ALPHA_OPAQUE;
};


class Mesh
{
protected:
	std::vector<vec3> positions;
	std::vector<vec3> normals;
	std::vector<vec2> texCoords;
	// each vertex can be influenced at most by 4 bones (vec4)
	std::vector<vec4> weights;
	std::vector<ivec4> influences; // joint ids of each vertex
	std::vector<unsigned int> indices; // vertex indices
	std::vector<MorphTarget> morphTargets;

	// for CPU skinning
	std::vector<vec3> skinnedPositions;
	std::vector<vec3> skinnedNormals;
	std::vector<mat4> poseMatrices; // global matrices of the pose joints

	// for render in the GPU
	Attribute<vec3>* posAttrib;
	Attribute<vec3>* normAttrib;
	Attribute<vec2>* uvAttrib;

	//for GPU skinning 
	Attribute<vec4>* weightsAttrib;
	Attribute<ivec4>* influencesAttrib;

	IndexBuffer* indexBuffer;

	DataTexture* morphTargetsAtlas;
	Material material;

public:
	Mesh();
	Mesh(const Mesh&);
	Mesh& operator=(const Mesh&);
	~Mesh();
	std::vector<char*> morphTargetNames;
	int* morphTargetsCount;
	std::string name;
	// getters
	std::vector<vec3>& getPositions();
	std::vector<vec3>& getNormals();
	std::vector<vec2>& getTexCoords();
	std::vector<vec4>& getWeights();
	std::vector<ivec4>& getInfluences();
	std::vector<unsigned int>& getIndices();
	std::vector<MorphTarget>& getMorphTargets();
	DataTexture* getMorphTargetsAtlas();
	Material& getMaterial();
	int getMorphTargetsCount();
	Texture* getTexture();
	// setter
	void setPositions(std::vector<vec3> positions);
	void setMaterial(Material material);
	
	// applies CPU mesh-skinning
	void CPUSkin(Skeleton& skeleton, Pose& pose);
	// syncs the vectors holding data to the GPU
	void updateOpenGLBuffers();
	void encodeMorphTargets();
	void bind(int position, int normal, int uv, int weight, int influence); // the parameters are the slots 
	void draw();
	void drawInstanced(unsigned int numInstances);
	void unBind(int position, int normal, int uv, int weight, int influence); // the parameters are the slots 

};
