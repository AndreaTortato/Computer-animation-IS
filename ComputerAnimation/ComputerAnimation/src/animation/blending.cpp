#include "blending.h"
#include "../lab3.h"

Pose makeAdditivePose(Skeleton& skeleton, Clip& clip) {
	Pose result = skeleton.getRestPose();
	clip.sample(result, clip.getStartTime());
	return result;
}

void add(Pose& output, Pose& inPose, Pose& addPose, Pose& basePose, int blendroot) {
	unsigned int numJoints = addPose.size();
	for (int i = 0; i < numJoints; ++i) {
		Transform input = inPose.getLocalTransform(i);
		Transform additive = addPose.getLocalTransform(i);
		Transform additiveBase = basePose.getLocalTransform(i);
		if (blendroot >= 0 && !isInHierarchy(addPose, blendroot, i)) {
			continue;
		}
		// outPose = inPose + (addPose - basePose)
		// [CA] To do: Compute the resulting transform
		Transform result; // result(position, rotation, scale)
		result.position = input.position + (additive.position - additiveBase.position);
		result.rotation = slerp(input.rotation, additive.rotation, 0.5); 
		result.scale = input.scale + (additive.scale - additiveBase.scale);

		output.setLocalTransform(i, result);
	}
}

bool isInHierarchy(Pose& pose, unsigned int parent, unsigned int search) {
	if (search == parent) {
		return true;
	}
	int p = pose.getParent(search);
	while (p >= 0) {
		if (p == (int)parent) {
			return true;
		}
		p = pose.getParent(p);
	}
	return false;
}