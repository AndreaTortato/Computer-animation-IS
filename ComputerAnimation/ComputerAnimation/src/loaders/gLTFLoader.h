#pragma once
#ifndef _H_GLTFLOADER_
#define _H_GLTFLOADER_
#include "../external/cgltf.h"
#include "../pose.h"
#include "../mesh.h"

cgltf_data* loadGLTFFile(const char* path);
void freeGLTFFile(cgltf_data* data);
Pose loadRestPose(const cgltf_data* data);
Pose loadBindPose(const cgltf_data* data);
std::vector<std::string> loadJointNames(const cgltf_data* data); 
Skeleton loadSkeleton(const cgltf_data* data);
std::vector<Mesh> loadMeshes(const cgltf_data* data);

namespace GLTFHelpers {
	Transform getLocalTransform(cgltf_node& node);
	int getNodeIndex(cgltf_node* target, cgltf_node* allNodes, unsigned int numNodes);
	void getScalarValues(std::vector<float>& out, unsigned int compCount, const cgltf_accessor& inAccessor);
	void meshFromAttribute(Mesh& outMesh, cgltf_attribute& attribute, cgltf_skin* skin, cgltf_node* nodes, unsigned int nodeCount);
};
#endif