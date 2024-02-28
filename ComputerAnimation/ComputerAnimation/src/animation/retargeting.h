#pragma once

#include "skeleton.h";
#include "../math/vec2.h"

class Retargeting {
private:
	Skeleton source; // identifier of source bone entity (should be in T-pose)
	Skeleton target; // identifier of target bone entity (should be in T-pose)

	std::vector<mat4> sourceInvBindMat;
	std::vector<mat4> targetInvBindMat;

	Pose targetBindPose;
	Pose *currentSourcePose;
	std::vector<int> map;

	vec2 leftLegChain;
	vec2 leftArmChain;
	vec2 rightLegChain;
	vec2 rightArmChain;
	vec2 spineChain;

	void automap();
	void fixBindPose();
public:
	Retargeting(Skeleton src, Skeleton tgt, Pose* current);
	void solve(Pose &targetPose);
};