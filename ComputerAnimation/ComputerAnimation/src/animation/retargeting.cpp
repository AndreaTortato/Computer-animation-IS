#include "retargeting.h"
#include <iostream>
#include <algorithm>
#include <regex>

Retargeting::Retargeting(Skeleton src, Skeleton tgt, Pose *current) {
	source = src; 
	target = tgt;
	currentSourcePose = current;
	sourceInvBindMat = src.getInvBindPose();
	targetBindPose = tgt.getBindPose();
	targetInvBindMat = tgt.getInvBindPose();

	// [CA] To do: (TASK 2) Map automatically the joints between the skeletons
	automap();

	// [CA] To do: (TASK 3&4) Fix the bind poses so they resemble a T-pose facing the Z axis
	fixBindPose();
};

// Replace special characters and simplify names
void renameAnimationBones(std::vector<std::string>& names) {
	std::regex expression("[-_:]");
	for (unsigned int i = 0; i < names.size(); i++) {
		names[i] = std::regex_replace(names[i], std::regex("mixamorig"), "");
		names[i] = std::regex_replace(names[i], expression, "");
	}
}

void Retargeting::automap() {
	// [CA] To do: Auto map source-target joints
	std::vector<std::string> sourceNames = source.getJointNames();
	std::vector<std::string> targetNames = target.getJointNames();

	renameAnimationBones(sourceNames);
	renameAnimationBones(targetNames);

	map.resize(sourceNames.size());

	for (int i = 0; i < sourceNames.size(); ++i) 
	{
		for (int j = 0; j < targetNames.size(); ++j)
		{
			if (targetNames[j] == sourceNames[i])
			{
				map[i] = j;
			}
		}	
	}
}

void Retargeting::fixBindPose() {

	Pose& srcBindPose = source.getBindPose();
	
	// [CA] To do: (TASK 4) Perform Auto Facing/Alingment to the src bind pose so it faces the +Z axis
	// ..
	
	// [CA] To do: (TASK 3) Perform Auto Posing so the src bind pose matches a T-pose
	// Make the left leg follow the -Y axis
	if (leftLegChain.x != -1 && leftLegChain.y != -1) 
	{
		vec3 leftLegDirection = srcBindPose.getGlobalTransform(leftLegChain.y).position - srcBindPose.getGlobalTransform(leftLegChain.x).position;
		vec3 targetLeftLegDirection = vec3(0, -1, 0);

		// Compute the rotation axis using the cross product
		vec3 rotationAxis = cross(leftLegDirection, targetLeftLegDirection);

		// Compute the rotation angle using the dot product
		float dotProduct = dot(leftLegDirection, targetLeftLegDirection);
		float legRotationAngle = acos(dotProduct);

		// Apply the rotation to the left leg chain
		for (int i = leftLegChain.x; i <= leftLegChain.y; ++i) {
			Transform transform = srcBindPose.getLocalTransform(i);
			transform.rotation = angleAxis(legRotationAngle, rotationAxis) * transform.rotation;
		}
	}
	
	// Make the right leg follow the -Y axis
	if (rightLegChain.x != -1 && rightLegChain.y != -1)
	{
		vec3 rightLegDirection = srcBindPose.getGlobalTransform(rightLegChain.y).position - srcBindPose.getGlobalTransform(rightLegChain.x).position;
		vec3 targetRightLegDirection = vec3(0, -1, 0);

		// Compute the rotation axis using the cross product
		vec3 rotationAxis = cross(rightLegDirection, targetRightLegDirection);

		// Compute the rotation angle using the dot product
		float dotProduct = dot(rightLegDirection, targetRightLegDirection);
		float legRotationAngle = acos(dotProduct);

		// Apply the rotation to the right leg chain
		for (int i = rightLegChain.x; i <= rightLegChain.y; ++i) {
			Transform transform = srcBindPose.getLocalTransform(i);
			transform.rotation = angleAxis(legRotationAngle, rotationAxis) * transform.rotation;
		}
	}
	
	// Make the left arm follow the +X axis
	if (leftArmChain.x != -1 && leftArmChain.y != -1)
	{
		vec3 leftArmDirection = srcBindPose.getGlobalTransform(leftArmChain.y).position - srcBindPose.getGlobalTransform(leftArmChain.x).position;
		vec3 targetLeftArmDirection = vec3(0, -1, 0);

		// Compute the rotation axis using the cross product
		vec3 rotationAxis = cross(leftArmDirection, targetLeftArmDirection);

		// Compute the rotation angle using the dot product
		float dotProduct = dot(leftArmDirection, targetLeftArmDirection);
		float armRotationAngle = acos(dotProduct);

		// Apply the rotation to the left arm chain
		for (int i = leftArmChain.x; i <= leftArmChain.y; ++i) {
			Transform transform = srcBindPose.getLocalTransform(i);
			transform.rotation = angleAxis(armRotationAngle, rotationAxis) * transform.rotation;
		}
	}
	
	// Make the right arm follow the -X axis
	if (rightArmChain.x != -1 && rightArmChain.y != -1)
	{
		vec3 rightArmDirection = srcBindPose.getGlobalTransform(rightArmChain.y).position - srcBindPose.getGlobalTransform(rightArmChain.x).position;
		vec3 targetRightArmDirection = vec3(0, -1, 0);

		// Compute the rotation axis using the cross product
		vec3 rotationAxis = cross(rightArmDirection, targetRightArmDirection);

		// Compute the rotation angle using the dot product
		float dotProduct = dot(rightArmDirection, targetRightArmDirection);
		float armRotationAngle = acos(dotProduct);

		// Apply the rotation to the right arm chain
		for (int i = rightArmChain.x; i <= rightArmChain.y; ++i) {
			Transform transform = srcBindPose.getLocalTransform(i);
			transform.rotation = angleAxis(armRotationAngle, rotationAxis) * transform.rotation;
		}
	}
	
	// set new T-pose
	source.set(srcBindPose, source.getRestPose(), source.getJointNames());
	sourceInvBindMat = source.getInvBindPose();
}

void Retargeting::solve(Pose& targetPose) {

	//automap();
	if (!map.size()) return;

	for (unsigned int sourceId = 0; sourceId < source.getBindPose().size(); sourceId++) {
		int targetId = map[sourceId];

		if (targetId <= 0) continue;

		std::string name = source.getJointName(sourceId);
		std::string Tname = target.getJointName(targetId);
		// std::cout << name << ", " << Tname << std::endl; //mapping in correct, for now the math below is the problem

		Transform srcWorldBindPose = source.getBindPose().getGlobalTransform(sourceId);
		Transform tgtWorldBindPose = target.getBindPose().getGlobalTransform(targetId);
		Transform localRetarget = target.getBindPose().getLocalTransform(targetId);

		// 1. Find Bind Rotation Difference from Source to Target
		quat srcRot = srcWorldBindPose.rotation;
		quat tgtRot = tgtWorldBindPose.rotation;
		quat rotationDifference = srcRot * inverse(tgtRot);

		// 2. Isolate the animated bone rotation by using Source parent's world bind rotation and Animated (current) local space rotation
		int sourceParentId = source.getBindPose().getParent(sourceId);
		quat srcParentWorld = source.getBindPose().getGlobalTransform(sourceParentId).rotation;
		quat srcLocalAnimated = currentSourcePose->getLocalTransform(sourceId).rotation;
		quat animatedBoneRot = inverse(srcParentWorld) * srcLocalAnimated;

		//3. Apply the bind rotation difference
		animatedBoneRot = animatedBoneRot * rotationDifference;

		// 4. Convert down the computed bone rotation to Target local space by using Target parent world bone
		int targetParentId = target.getBindPose().getParent(targetId);
		quat tgtParentWorld = target.getBindPose().getGlobalTransform(targetParentId).rotation;
		quat targetLocalRot = tgtParentWorld * inverse(animatedBoneRot);

		// Apply the new rotation to the retargeted local transform
		localRetarget.rotation = targetLocalRot;

		// If the joint is the hips, retarget its position
		if (name == "Hips" || name == "mixamorig:Hips") {
			vec3 srcBindPos = source.getBindPose().getGlobalTransform(sourceId).position;
			vec3 srcCurrPos = currentSourcePose->getGlobalTransform(sourceId).position;
			vec3 tgtBindPos = target.getBindPose().getGlobalTransform(targetId).position;

			// Check that the division is not done with a 0
			srcBindPos.x = srcBindPos.x == 0.0 ? 1.0 : srcBindPos.x;
			srcBindPos.y = srcBindPos.y == 0.0 ? 1.0 : srcBindPos.y;
			srcBindPos.z = srcBindPos.z == 0.0 ? 1.0 : srcBindPos.z;

			// Compute new hips position
			vec3 newHipsPos = srcCurrPos - srcCurrPos;

			// Convert it from global to local space
			newHipsPos = tgtBindPos + newHipsPos;

			// Apply the new position to the retargeted local transform
			//localRetarget.position = newHipsPos;
		}

		// Set the retargeted transformation to the evaluated joint of the target pose
		targetPose.setLocalTransform(targetId, localRetarget);
	}
}