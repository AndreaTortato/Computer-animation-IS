#include "pose.h"
#include <iostream>

Pose::Pose() { }

Pose::Pose(unsigned int numJoints) {
	resize(numJoints);
}

Pose::Pose(const Pose& p) {
	*this = p;
}

// resize arrays
void Pose::resize(unsigned int size) {
	// TO DO:
	joints.resize(size);
	parents.resize(size);
}

// get the number of joints
unsigned int Pose::size() {
	// TO DO:
	return joints.size();
}

// set parent id
void Pose::setParent(unsigned int id, unsigned int parentId) {
	// TO DO:
	parents[id] = parentId;
}

// get parent id
int Pose::getParent(unsigned int id) {
	// TO DO:
	return parents[id];
}

//set local transform of the joint
void Pose::setLocalTransform(unsigned int id, const Transform& transform) {
	//TO DO:
	joints[id] = transform;
}

// get local transform of the joint
Transform Pose::getLocalTransform(unsigned int id) {
	// TO DO:
	return joints[id];
}


// get global (world) transform of the joint
Transform Pose::getGlobalTransform(unsigned int id) {
	// TO DO: use "combine()" function to combine two transforms
	Transform globalTransform = joints[id];
	
	for (int parent = getParent(id); parent >= 0; parent = getParent(parent))
	{
		globalTransform = combine(joints[parent], globalTransform);
	}

	return globalTransform;
}

// get global matrices of the joints
std::vector<mat4> Pose::getGlobalMatrices() {
	unsigned int numJoints = size();
	std::vector<mat4> out(numJoints);

    Transform globalTransform;
    mat4 globalTransformMatrix;

	// TO DO: For every joint, find the global transform (using getGlobalTransform), convert it into a matrix, and store the result in a vector of matrices
	for (unsigned int i = 0; i < numJoints; ++i) 
	{
		globalTransform = getGlobalTransform(i);
		globalTransformMatrix = transformToMat4(globalTransform);
		out[i] = globalTransformMatrix;
	}

	return out;
}