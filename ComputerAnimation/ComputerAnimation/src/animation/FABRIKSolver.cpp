#include "FABRIKSolver.h"

void FABRIKSolver::resize(unsigned int newSize) {
	// [CA] To do: Resize all arrays
	
}

// Converts the chain from local joints transforms to global transforms
void FABRIKSolver::ikChainToWorld() {
	unsigned int chainSize = size();
	for (unsigned int i = 0; i < chainSize; ++i) {
		Transform world = getGlobalTransform(i);
		worldChain[i] = world.position;
		if (i >= 1) {
			vec3 prev = worldChain[i - 1];
			lengths[i] = len(world.position - prev);
		}
	}
	if (chainSize > 0) {
		lengths[0] = 0.0f;
	}
}

//Converts the chain from global joints transforms to local transforms
void FABRIKSolver::worldToIKChain() {
	unsigned int chainSize = size();
	if (chainSize == 0) { return; }
	for (unsigned int i = 0; i < chainSize - 1; ++i) {
		Transform world = getGlobalTransform(i);
		Transform next = getGlobalTransform(i + 1);
		vec3 position = world.position;
		quat rotation = world.rotation;

		vec3 toNext = next.position - position;
		toNext = inverse(rotation) * toNext;
		vec3 toDesired = worldChain[i + 1] - position;
		toDesired = inverse(rotation) * toDesired;

		quat delta = fromTo(toNext, toDesired);
		ikChain[i].rotation = delta * ikChain[i].rotation;
	}
}

void FABRIKSolver::iterateBackward(const vec3 & goal) {
	// [CA] To do:
	// Set the end-effector to the goal location

	// For each joint in the chain (reversed) starting from end-effector - 1:
		// 1. Reposition the joint using the direction (unitary vector) to the next joint takio

}

void FABRIKSolver::iterateForward(const vec3& base) {
	// [CA] To do:
	// Set the first joint (origin) of the chain to the origin location

	// For each joint in the chain starting from 1:
		// 1. Reposition the joint using the direction (unitary vector) to the previous joint
	
}

bool FABRIKSolver::solve(const Transform& target) {
	// Local variables and size check
	unsigned int chainSize = size();
	if (chainSize == 0) { return false; }
	unsigned int last = chainSize - 1;
	float thresholdSq = threshold * threshold;

	ikChainToWorld();
	vec3 goal = target.position;
	vec3 base = worldChain[0];

	// [CA] To do: 
	// For each iteration of the algorithm:
		// 1. Check if the end-effector has reached the goal
		// 2. Perform backward step
		// 3. Perform forward step
		// 4. Apply constraints if required:
		//		4.1. Convert the chain in local space
		//		4.2. Apply constraints
		//		4.3. Convert the chain in global space again
	// Convert the chain in local space again
	// Last check if end-effector has reached the goal

	return false;
}