#pragma once

#include "track.h"
#include "../math/transform.h"

// binds a transform to a joint
class TransformTrack {
protected:
	unsigned int id; // joint Id
	VectorTrack position;
	QuaternionTrack rotation;
	VectorTrack scale;
public:
	TransformTrack();
	unsigned int getId();
	void setId(unsigned int id);
	VectorTrack& getPositionTrack();
	QuaternionTrack& getRotationTrack();
	VectorTrack& getScaleTrack();
	float getStartTime();
	float getEndTime();
	bool isValid();
	Transform sample(const Transform& ref, float time, bool looping);
};