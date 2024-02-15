#include "CCDSolver.h"

bool CCDSolver::solve(const Transform& target) {
	// Local variables and chainSize check
	unsigned int chainSize = size();
	if (chainSize == 0) { return false; }
	unsigned int last = chainSize - 1; // end effector
	float thresholdSq = threshold * threshold;
	vec3 goal = target.position;

	// [CA] To do:
	// For each numSteps:
	for (int step = 0; step < numSteps; ++step)
	{
		// 1. Check if the end-effector has reached the goal (using the lenSq)
		vec3 dist = ikChain[last].position - goal;
		if (lenSq(dist) < thresholdSq)	return true;

		// 2. For each joint in the chain (reversed) starting from end-effector - 1:
		for (int j = last - 1; j >= 0; --j)
		{
			// 2.1. Find a vector from current joint to end effector
			vec3 jointToEndEffector = ikChain[last].position - ikChain[j].position;
			normalize(jointToEndEffector);

			// 2.2. Find a vector from the current joint to the goal
			vec3 jointToGoal = goal - ikChain[j].position;
			normalize(jointToGoal);

			// 2.3. Find the rotation between these two directions
			quat rotation = lookRotation(jointToEndEffector, jointToGoal);

			// 2.4. Rotate the joint so the joint to effector vector matches the orientation of the joint to goal vector
			ikChain[j].rotation = rotation * ikChain[j].rotation;

			// 2.5. Apply constraints if required (TASK 4)

			// 2.6. Check if the end-effector has reached the goal
			vec3 dist = ikChain[last].position - goal;
			if (lenSq(dist) < thresholdSq)	return true;
		}
	}

	return false;
} // End CCDSolver::Solve function
