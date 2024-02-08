#include "clip.h"

Clip::Clip() {
	name = "No name given";
	startTime = 0.0f;
	endTime = 0.0f;
	looping = true;
}

float Clip::sample(Pose& outPose, float time) {
	if (getDuration() == 0.0f) {
		return 0.0f;
	}
	time = adjustTimeToFitRange(time);
	unsigned int size = tracks.size();
	for (unsigned int i = 0; i < size; ++i) {
		// get the joint ID of the  track
		unsigned int j = tracks[i].getId(); // Joint
		Transform local = outPose.getLocalTransform(j);
		// sample the track
		Transform animated = tracks[i].sample(local, time, looping);
		// assign the sampled value back to the Pose reference
		outPose.setLocalTransform(j, animated);
	}
	return time;
}

float Clip::adjustTimeToFitRange(float inTime) {
	if (looping) {
		float duration = endTime - startTime;
		if (duration <= 0) { 0.0f; }
		inTime = fmodf(inTime - startTime, endTime - startTime);
		if (inTime < 0.0f) {
			inTime += endTime - startTime;
		}
		inTime = inTime + startTime;
	}
	else {
		if (inTime < startTime) {
			inTime = startTime;
		}
		if (inTime > endTime) {
			inTime = endTime;
		}
	}
	return inTime;
}

void Clip::recalculateDuration() {
	startTime = 0.0f;
	endTime = 0.0f;
	bool startSet = false;
	bool endSet = false;
	unsigned int tracksSize = tracks.size();
	for (unsigned int i = 0; i < tracksSize; ++i) {
		if (tracks[i].isValid()) {
			float minStartTime = tracks[i].getStartTime();
			float maxEndTime = tracks[i].getEndTime();
			if (minStartTime < startTime || !startSet) {
				startTime = minStartTime;
				startSet = true;
			}
			if (maxEndTime > endTime || !endSet) {
				endTime = maxEndTime;
				endSet = true;
			}
		}
	}
}

// retrieves the TransformTrack object for a specific joint in the clip
TransformTrack& Clip::operator[](unsigned int joint) {
	for (int i = 0, s = tracks.size(); i < s; ++i) {
		if (tracks[i].getId() == joint) {
			return tracks[i];
		}
	}
	// if no qualifying track is found, a new one is created and returned
	tracks.push_back(TransformTrack());
	tracks[tracks.size() - 1].setId(joint);
	return tracks[tracks.size() - 1];
}

// getters
std::string& Clip::getName() {
	return name;
}

unsigned int Clip::getIdAtIndex(unsigned int index) {
	return tracks[index].getId();
}

unsigned int Clip::size() {
	return (unsigned int)tracks.size();
}

float Clip::getDuration() {
	return endTime - startTime;
}

float Clip::getStartTime() {
	return startTime;
}

float Clip::getEndTime() {
	return endTime;
}

bool Clip::getLooping() {
	return looping;
}

// setters
void Clip::setName(const std::string& inNewName) {
	name = inNewName;
}

void Clip::setIdAtIndex(unsigned int index, unsigned int id) {
	return tracks[index].setId(id);
}

void Clip::setLooping(bool inLooping) {
	looping = inLooping;
}