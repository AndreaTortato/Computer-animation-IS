#pragma once
#include <vector>
#include <string>
#include "transformTrack.h"
#include "pose.h"

class Clip {
protected:
	std::vector<TransformTrack> tracks;
	std::string name;
	float startTime;
	float endTime;
	bool looping;

protected:
	float adjustTimeToFitRange(float inTime);

public:
	Clip();

	//gets joint Id based for a specific track index
	unsigned int getIdAtIndex(unsigned int index);
	//sets joint ID based on the index of the joint in the clip
	void setIdAtIndex(unsigned int idx, unsigned int id);
	unsigned int size();

	//samples the animation clip at the provided time into the Pose reference
	float sample(Pose& outPose, float inTime);
	//returns a transform track for the specified joint
	TransformTrack& operator[](unsigned int index);

	//sets the start/end time of the animation clip based on the tracks that make up the clip
	void recalculateDuration();

	std::string& getName();
	void setName(const std::string& inNewName);
	float getDuration();
	float getStartTime();
	float getEndTime();
	bool getLooping();
	void setLooping(bool inLooping);
};
