#pragma once
#pragma once

#include "../application.h"
#include "../camera.h"
#include <vector>
#include "../external/cgltf.h"
#include "../shading/debugDraw.h"
#include "../shading/mesh.h"
#include "../shading/texture.h"
#include "../animation/pose.h"
#include "../animation/retargeting.h"
#include "../animation/clip.h"
#include "../math/mat4.h"

class Lab5 : public Application {
	struct AnimationInstance {
		Pose animatedPose;
		std::vector <mat4> posePalette;
		unsigned int clip;
		float playback;
		float speed;

		inline AnimationInstance() : clip(0), playback(0.0f), speed(1.0f) { }
	};

	struct Entity {
		std::vector<Mesh> meshes;
		std::vector<Clip> clips;
		Skeleton skeleton;
		DebugDraw* skeletonHelper = NULL;
		mat4 model;
	};
protected:
	enum tasks { TASK1, TASK2, TASK3 };
	static const char* tasks[];

	int currentTask;
	const char** clips;
	unsigned int numUIClips;

	float currentTime;

	int showAxes;
	int showSkeleton;
	int showBindPose;
	int showTarget;
	int showBadTarget;
	int showSource;

	DebugDraw* mUpAxis;
	DebugDraw* mRightAxis;
	DebugDraw* mForwardAxis;
	DebugDraw* badSkeletonHelper;

	Camera* camera;
	bool dragging = false;
	bool moving = false;


	Shader* shader;
	
	// Source characters
	Entity sourceGLTF;
	Entity sourceBVH;
	 
	// Target character
	Entity target;
	Pose badTargetPose; // target pose without retargeting
	Pose targetPose; // target pose with retargeting

	AnimationInstance animationInfo;
	Retargeting* retargetingSolver;

	Skeleton bvhSkeleton;
	Clip bvhClip;

public:
	void init();

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


