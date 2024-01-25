#include "skeleton.h"

Skeleton::Skeleton() {}

Skeleton::Skeleton(const Pose& rest, const Pose& bind, const std::vector<std::string>& names) {
	set(rest, bind, names);
}

void Skeleton::set(const Pose& rest, const Pose& bind, const std::vector<std::string>& names) {
	restPose = rest;
	bindPose = bind;
	jointNames = names;
	// TODO: any time the bind pose of the skeleton is updated, the inverse bind pose should be re - calculated as well.
	updateInvBindPose();
}

Pose& Skeleton::getBindPose() {
	// TO DO:
	return bindPose;
}

Pose& Skeleton::getRestPose() {
	// TO DO:
	return restPose;
}

std::vector<mat4>& Skeleton::getInvBindPose() {
	return invBindPose;
}

std::vector<std::string>& Skeleton::getJointNames() {
	return jointNames;
}

std::string& Skeleton::getJointName(unsigned int id) {
	// TO DO:
	return jointNames[id];
}

void Skeleton::updateInvBindPose() {
	// TO DO: Get the world space transform of each joint, convert it into a matrix and invert it, then update the inverse bind pose matrix of the joint 
	unsigned int numJoints = bindPose.size();
	invBindPose.resize(numJoints);

	for (unsigned int i = 0; i < numJoints; ++i) 
	{
		Transform worldTransform = bindPose.getGlobalTransform(i);
		mat4 invBindPoseMatrix = inverse(transformToMat4(worldTransform));
		invBindPose[i] = invBindPoseMatrix;
	}
}