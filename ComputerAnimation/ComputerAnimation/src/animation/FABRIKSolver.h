#pragma once

#include <vector>
#include "../math/transform.h"
#include "ikSolver.h"

class FABRIKSolver : public IKSolver {
protected:
	std::vector<vec3> worldChain;
	std::vector<float> lengths;
protected:
	void ikChainToWorld();
	void iterateForward(const vec3& goal);
	void iterateBackward(const vec3& base);
	void worldToIKChain();
public:
	void resize(unsigned int newSize);
	bool solve(const Transform& target);
};
