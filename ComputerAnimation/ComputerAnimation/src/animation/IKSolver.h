#pragma once

#include <vector>
#include "../math/transform.h"

class IKSolver {
protected:
	std::vector<Transform> ikChain;
	std::vector<unsigned int> jointIndices;
	unsigned int numSteps;
	float threshold;
public:
	IKSolver();

	unsigned int size();
	virtual void resize(unsigned int newSize);
	Transform& operator[](unsigned int index);

	std::vector<Transform> getChain();
	std::vector<unsigned int> getJointIndices();
	void setChain(std::vector<Transform> chain);
	Transform& getLocalTransform(unsigned int index);
	void setLocalTransform(unsigned int index, const Transform& t);
	void setLocalTransform(unsigned int index, const Transform& t, unsigned int jointIdx);

	Transform getGlobalTransform(unsigned int index);
	unsigned int getNumSteps();
	void setNumSteps(unsigned int numSteps);
	float getThreshold();
	void setThreshold(float value);

	virtual bool solve(const Transform& target) { return false; };
};