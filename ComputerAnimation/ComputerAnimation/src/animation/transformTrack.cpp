#include "transformTrack.h"

TransformTrack::TransformTrack() {
	id = 0;
}

unsigned int TransformTrack::getId() {
	return id;
}

void TransformTrack::setId(unsigned int index) {
	id = index;
}

VectorTrack& TransformTrack::getPositionTrack() {
	return position;
}

QuaternionTrack& TransformTrack::getRotationTrack() {
	return rotation;
}

VectorTrack& TransformTrack::getScaleTrack() {
	return scale;
}

bool TransformTrack::isValid() {
	return position.size() > 1 ||
		rotation.size() > 1 ||
		scale.size() > 1;
}

float TransformTrack::getStartTime() {
	float result = 0.0f;
	bool isSet = false;
	if (position.size() > 1) {
		result = position.getStartTime();
		isSet = true;
	}
	if (rotation.size() > 1) {
		float rotationStart = rotation.getStartTime();
		if (rotationStart < result || !isSet) {
			result = rotationStart;
			isSet = true;
		}
	}
	if (scale.size() > 1) {
		float scaleStart = scale.getStartTime();
		if (scaleStart < result || !isSet) {
			result = scaleStart;
			isSet = true;
		}
	}
	return result;
}

float TransformTrack::getEndTime() {
	float result = 0.0f;
	bool isSet = false;
	if (position.size() > 1) {
		result = position.getEndTime();
		isSet = true;
	}
	if (rotation.size() > 1) {
		float rotationEnd = rotation.getEndTime();
		if (rotationEnd > result || !isSet) {
			result = rotationEnd;
			isSet = true;
		}
	}
	if (scale.size() > 1) {
		float scaleEnd = scale.getEndTime();
		if (scaleEnd > result || !isSet) {
			result = scaleEnd;
			isSet = true;
		}
	}
	return result;
}

// only samples one of its component tracks if that track has two or more frames
Transform TransformTrack::sample(const Transform& ref, float time, bool loop) {
	// If one of the transform components isn't animated by the transform track, the value of the reference transform is used 
	Transform result = ref; // Assign default values
	if (position.size() > 1) { // Only if valid
		result.position = position.sample(time, loop);
	}
	if (rotation.size() > 1) { // Only if valid
		result.rotation = rotation.sample(time, loop);
	}
	if (scale.size() > 1) { // Only if valid
		result.scale = scale.sample(time, loop);
	}
	return result;
}