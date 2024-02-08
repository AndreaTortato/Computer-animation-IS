#pragma once

#include "frame.h"
#include <vector>
#include "../math/vec3.h"
#include "../math/quat.h"

enum class Interpolation {
	Constant,
	Linear,
	Cubic
};

// Collection of frames
template<typename T, int N>
class Track {
protected:
	std::vector<Frame<N>> frames;
	Interpolation interpolation; // interpolation type
public:
	Track();
	void resize(unsigned int size);
	unsigned int size();
	Interpolation getInterpolation();
	void setInterpolation(Interpolation interp);
	float getStartTime();
	float getEndTime();
	// parameters: time value, if the track is looping or not
	T sample(float time, bool looping);
	Frame<N>& operator[](unsigned int index);
protected:
	// helper functions, a sample for each type of interpolation
	T sampleConstant(float time, bool looping);
	T sampleLinear(float time, bool looping);
	T sampleCubic(float time, bool looping);
	// helper function to evaluate Hermite splines (tangents)
	T hermite(float time, const T& p1, const T& s1, const T& _p2, const T& s2);
	T bezier(float t, const T& p1, const T& c1, const T& _p2, const T& c2);
	int frameIndex(float time, bool looping);
	// takes an input time that is outside the range of the track and adjusts it to be a valid time on the track
	float adjustTimeToFitTrack(float t, bool loop);

	// casts an array of floats (the data inside a frame) to the templated type of the track	
	T cast(float* value); // Will be specialized
};

typedef Track<float, 1> ScalarTrack;
typedef Track<vec3, 3> VectorTrack;
typedef Track<quat, 4> QuaternionTrack;