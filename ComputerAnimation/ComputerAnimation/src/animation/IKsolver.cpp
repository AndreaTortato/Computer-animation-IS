#include "IKSolver.h"

IKSolver::IKSolver() {
	numSteps = 15;
	threshold = 0.00001f;
}

unsigned int IKSolver::size() {
	return ikChain.size();
}

void IKSolver::resize(unsigned int newSize) {
	ikChain.resize(newSize);
	jointIndices.resize(newSize);
}

Transform& IKSolver::operator[](unsigned int index) {
	return ikChain[index];
}

std::vector<Transform> IKSolver::getChain() {
	return ikChain;
}

std::vector<unsigned int> IKSolver::getJointIndices() {
	return jointIndices;
}

void IKSolver::setChain(std::vector<Transform> chain) {
	ikChain = chain;
	resize(ikChain.size());
}

Transform& IKSolver::getLocalTransform(unsigned int index) {
	return ikChain[index];
}

void IKSolver::setLocalTransform(unsigned int index, const Transform& t) {
	ikChain[index] = t;
}

void IKSolver::setLocalTransform(unsigned int index, const Transform& t, unsigned int jointIdx) {
	ikChain[index] = t;
	jointIndices[index] = jointIdx;
}

unsigned int IKSolver::getNumSteps() {
	return numSteps;
}

void IKSolver::setNumSteps(unsigned int steps) {
	numSteps = steps;
}

float IKSolver::getThreshold() {
	return threshold;
}

void IKSolver::setThreshold(float value) {
	threshold = value;
}

Transform IKSolver::getGlobalTransform(unsigned int index) {
	unsigned int size = (unsigned int)ikChain.size();
	Transform world = ikChain[index];
	for (int i = (int)index - 1; i >= 0; --i) {
		world = combine(ikChain[i], world);
	}
	return world;
}