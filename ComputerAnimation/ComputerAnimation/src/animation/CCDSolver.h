#pragma once

#include <vector>
#include "../math/transform.h"
#include "IKSolver.h"

class CCDSolver : public IKSolver {
	protected:
		Transform auxParent; // in global space
	public:
		bool solve(const Transform& target);
};