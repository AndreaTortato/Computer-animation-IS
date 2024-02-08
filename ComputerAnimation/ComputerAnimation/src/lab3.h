#pragma once

#include "../application.h"
#include "../camera.h"
#include <vector>
#include "../external/cgltf.h"
#include "../shading/debugDraw.h"
#include "../shading/mesh.h"
#include "../shading/texture.h"
#include "../animation/pose.h"
#include "../animation/track.h"
#include "../animation/clip.h"

struct AnimationInstance {
	Pose animatedPose;
	std::vector <mat4> poseMatrices;
	unsigned int clip;
	float playback;
	Transform model;

	inline AnimationInstance() : clip(0), playback(0.0f) { }
};

class Lab3 : public Application {
protected:
	enum tasks { TASK1, TASK2, TASK3, TASK4, TASK5 };
	static const char* tasks[];
	static const char* interpolation[];

	int currentTask;
	int interpolationType;
	int showAxes;
	int showSkeleton;

	DebugDraw* mUpAxis;
	DebugDraw* mRightAxis;
	DebugDraw* mForwardAxis;
	DebugDraw* poseHelper;

	Camera* camera;
	bool dragging = false;
	bool moving = false;

	Shader* shader;

	std::vector<Clip> clips;
	std::vector<Mesh> meshes;
	Skeleton skeleton;
	Texture* tex;
	AnimationInstance animInfo;
	float playbackTime;

	// TASK 1 & 2
	Transform objectTransform;
	VectorTrack track;
	float duration;
	float currentTime;
	int selectedFrame;

	// TASK 3
	Clip clip;

	// TASK 4
	Pose addPose;
	Pose additiveBase;
	float additiveTime;
	unsigned int additiveIndex;

public:
	void init();
	VectorFrame makeVectorFrame(float time, const vec3& in, const vec3& value, const vec3& out);
	QuaternionFrame makeQuaternionFrame(float time, const quat& in, const quat& value, const quat& out);
	void update(float inDeltaTime);
	void render(float inAspectRatio);
	void ImGui(nk_context* context);
	void shutdown();

	// Input event callbacks
	void onKeyDown(int key, int scancode);
	void onKeyUp(int key, int scancode);
	void onRightMouseButtonDown();
	void onRightMouseButtonUp();
	void onLeftMouseButtonDown();
	void onLeftMouseButtonUp();
	void onMiddleMouseButtonDown();
	void onMiddleMouseButtonUp();
	void onScroll(double xOffset, double yOffset);
};
