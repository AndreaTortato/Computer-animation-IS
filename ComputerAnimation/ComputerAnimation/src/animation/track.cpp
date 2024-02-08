#include "track.h"

template Track<float, 1>;
template Track<vec3, 3>;
template Track<quat, 4>;

// This class is templated, so some functions are declared but not necessary implemented depending of the interpolation type

// Track helpers avoid having to make specialized versions of the interpolation functions
namespace TrackHelpers {

	// linear interpolation for each type of track
	inline float interpolate(float a, float b, float t) {
		return a + (b - a) * t;
	}
	inline vec3 interpolate(const vec3& a, const vec3& b, float t) {
		return lerp(a, b, t);
	}
	inline quat interpolate(const quat& a, const quat& b, float t) {
		quat result = mix(a, b, t);
		if (dot(a, b) < 0) { // Neighborhood
			result = mix(a, -b, t);
		}
		return normalized(result); // NLerp, not slerp
	}

	// When a Hermite spline is interpolated, if the input type was a quaternion, the result needs to be normalized.
	inline float adjustCurveResult(float f) {
		return f;
	}
	inline vec3 adjustCurveResult(const vec3& v) {
		return v;
	}
	inline quat adjustCurveResult(const quat& q) {
		return normalized(q);
	}

	// Make sure two quaternions are in the correct neighborhood
	inline void neighborhood(const float& a, float& b) { }
	inline void neighborhood(const vec3& a, vec3& b) { }
	inline void neighborhood(const quat& a, quat& b) {
		if (dot(a, b) < 0) {
			b = -b;
		}
	}
}; // End Track Helpers namespace

template<typename T, int N>
Track<T, N>::Track() {
	interpolation = Interpolation::Linear;
}

template<typename T, int N>
float Track<T, N>::getStartTime() {
	return frames[0].time;
}

template<typename T, int N>
float Track<T, N>::getEndTime() {
	return frames[frames.size() - 1].time;
}

// call sampleConstant, sampleLinear, or sampleCubic, depending on the track type.
template<typename T, int N>
T Track<T, N>::sample(float time, bool looping) {
	if (interpolation == Interpolation::Constant) {
		return sampleConstant(time, looping);
	}
	else if (interpolation == Interpolation::Linear) {
		return sampleLinear(time, looping);
	}

	return sampleCubic(time, looping);
}

template<typename T, int N>
Frame<N>& Track<T, N>::operator[](unsigned int index) {
	return frames[index];
}

// size of the frames vector
template<typename T, int N>
void Track<T, N>::resize(unsigned int size) {
	frames.resize(size);
}

template<typename T, int N>
unsigned int Track<T, N>::size() {
	return frames.size();
}

template<typename T, int N>
Interpolation Track<T, N>::getInterpolation() {
	return interpolation;
}

template<typename T, int N>
void Track<T, N>::setInterpolation(Interpolation interpolationType) {
	interpolation = interpolationType;
}

template<typename T, int N>
T Track<T, N>::hermite(float t, const T& p1, const T& s1, const T& _p2, const T& s2) {
	T p2 = _p2;
	TrackHelpers::neighborhood(p1, p2); // choose the short path for rotations
	// [CA] To do: complete this function using the basis functions
	float h_00 = (1.0f - 2.0f * t) * pow(1.0f - t, 2);
	float h_01 = pow(t, 2) * (3.0f - 2.0f * t);
	float h_10 = t * pow((1.0f - t), 2);
	float h_11 = pow(t, 2) * (t - 1.0f);
	T result = p1 * h_00 + s1 * h_10 + p2 * h_01 + s2 * h_11;
	return TrackHelpers::adjustCurveResult(result); // normalize quaternions to make them unitary
}

template<typename T, int N>
T Track<T, N>::bezier(float t, const T& p1, const T& c1, const T& _p2, const T& c2) {
	T p2 = _p2;
	TrackHelpers::neighborhood(p1, p2); // choose the short path for rotations
	// [CA] To do: complete this function using the basis functions
	float h_00 = pow((1.0f - t), 3);
	float h_01 = 3.0f * pow((1.0f - t), 2) * t;
	float h_10 = 3.0f * (1.0f - t) * pow(t, 2);
	float h_11 = pow(t, 3);
	T result = p1 * h_00 + c1 * h_01 + c2 * h_10 + p2 * h_11; 
	return TrackHelpers::adjustCurveResult(result); // normalize quaternions to make them unitary
}

// return the frame immediately before that time (on the left)
template<typename T, int N>
int Track<T, N>::frameIndex(float time, bool looping) {
	unsigned int size = (unsigned int)frames.size();
	if (size <= 1) {
		return -1;
	}
	// If the track is sampled as looping, the input time needs to be adjusted so that it falls between the start and end frames.
	if (looping) {
		float startTime = frames[0].time;
		float endTime = frames[size - 1].time;
		float duration = endTime - startTime;
		time = fmodf(time - startTime, endTime - startTime);
		// looping, time needs to be adjusted so that it is within a valid range.
		if (time < 0.0f) {
			time += endTime - startTime;
		}
		time = time + startTime;
	}
	else {
		// clamp the time in the track frames range
		if (time <= frames[0].time) {
			return 0;
		}
		if (time >= frames[size - 2].time) {
			// The Sample function always needs a current and next frame (for interpolation), so the index of the second-to-last frame is used.
			return (int)size - 2;
		}
	}
	for (int i = (int)size - 1; i >= 0; --i) {
		if (time >= frames[i].time) {
			return i;
		}
	}
	// Invalid code, we should not reach here!
	return -1;
} // End of frameIndex

// Adjusts the time to be in the range of the start/end frames of the track.
template<typename T, int N>
float Track<T, N>::adjustTimeToFitTrack(float time, bool looping) {
	unsigned int size = (unsigned int)frames.size();
	if (size <= 1) {
		return 0.0f;
	}
	float startTime = frames[0].time;
	float endTime = frames[size - 1].time;
	float duration = endTime - startTime;
	if (duration <= 0.0f) {
		return 0.0f;
	}
	if (looping) {
		time = fmodf(time - startTime, endTime - startTime);
		if (time < 0.0f) {
			time += endTime - startTime;
		}
		time = time + startTime;
	}

	else {
		if (time <= frames[0].time) {
			time = startTime;
		}
		if (time >= frames[size - 1].time) {
			time = endTime;
		}
	}
	return time;
}

// They cast a float array stored in a Frame class into the data type that the Frame class represents
template<> float Track<float, 1>::cast(float* value) {
	return value[0];
}

template<> vec3 Track<vec3, 3>::cast(float* value) {
	return vec3(value[0], value[1], value[2]);
}

template<> quat Track<quat, 4>::cast(float* value) {
	quat r = quat(value[0], value[1], value[2], value[3]);
	return normalized(r);
}

// often used for things such as visibility flags, where it makes sense for the value of a variable to change from one frame to the next without any real interpolation
template<typename T, int N>
T Track<T, N>::sampleConstant(float t, bool loop) {
	int frame = frameIndex(t, loop);
	if (frame < 0 || frame >= (int)frames.size()) {
		return T();
	}
	return cast(&frames[frame].value[0]);
}

// applications provide an option to approximate animation curves by sampling them at set intervals
template<typename T, int N>
T Track<T, N>::sampleLinear(float time, bool looping) {
	int thisFrame = frameIndex(time, looping);
	if (thisFrame < 0 || thisFrame >= frames.size() - 1) {
		return T();
	}
	int nextFrame = thisFrame + 1;
	// make sure the time is valid
	float trackTime = adjustTimeToFitTrack(time, looping);
	float thisTime = frames[thisFrame].time;
	float frameDelta = frames[nextFrame].time - thisTime;
	if (frameDelta <= 0.0f) {
		return T();
	}
	float t = (trackTime - thisTime) / frameDelta;
	T start = cast(&frames[thisFrame].value[0]);
	T end = cast(&frames[nextFrame].value[0]);
	return TrackHelpers::interpolate(start, end, t);
}

template<typename T, int N>
T Track<T, N>::sampleCubic(float time, bool looping) {
	int thisFrame = frameIndex(time, looping);
	if (thisFrame < 0 || thisFrame >= frames.size() - 1) {
		return T();
	}
	int nextFrame = thisFrame + 1;

	float trackTime = adjustTimeToFitTrack(time, looping);
	float thisTime = frames[thisFrame].time;
	float frameDelta = frames[nextFrame].time - thisTime;
	if (frameDelta <= 0.0f) {
		return T();
	}

	// cast function normalizes quaternions, which is bad because slopes are not meant to be quaternions.
	// Using memcpy instead of cast copies the values directly, avoiding normalization.
	float t = (trackTime - thisTime) / frameDelta;
	size_t fltSize = sizeof(float);
	T point1 = cast(&frames[thisFrame].value[0]);
	T slope1;// = frames[thisFrame].out * frameDelta;
	memcpy(&slope1, frames[thisFrame].out, N * fltSize); // memcpy instead of cast to avoid normalization
	slope1 = slope1 * frameDelta;
	T point2 = cast(&frames[nextFrame].value[0]);
	T slope2;// = frames[nextFrame].in[0] * frameDelta;
	memcpy(&slope2, frames[nextFrame].in, N * fltSize);
	slope2 = slope2 * frameDelta;

	return hermite(t, point1, slope1, point2, slope2); // [CA] To do: call the hermite or bezier methods
	//return bezier(t, point1, slope1, point2, slope2);
}