#pragma once
#include "pose.h"
#include <string>

class Skeleton
{
protected:
	Pose bindPose;
	Pose restPose;
	
	std::vector<mat4> invBindPose; // vector of inverse bind pose matrix of each joint
	std::vector<std::string> jointNames; // vector of the name of each joint

	// updates the inverse bind pose matrices: any time the bind pose of the skeleton is updated, the inverse bind pose should be re-calculated as well
	void updateInvBindPose();
public:

	Skeleton(); // Empty constructor
	// Initialize the skeleton given the rest and bind poses, and the names of the joints
	Skeleton(const Pose& rest, const Pose& bind, const std::vector<std::string>& names);

	void set(const Pose& rest, const Pose& bind, const std::vector<std::string>& names);

	Pose& getBindPose();
	Pose& getRestPose();

	std::vector<mat4>& getInvBindPose();
	std::vector<std::string>& getJointNames();
	std::string& getJointName(unsigned int id);
};