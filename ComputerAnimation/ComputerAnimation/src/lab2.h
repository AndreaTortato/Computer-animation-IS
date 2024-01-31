#pragma once

#include "application.h"
#include "shading/debugDraw.h"
#include "pose.h"
#include "skeleton.h"
#include "camera.h"
#include "shading/texture.h"
#include "shading/shader.h"
#include "mesh.h"
#include "cgltf.h"

class Lab2 : public Application {
protected:
	enum tasks { TASK1, TASK2, TASK3, TASK4, TASK5 };
	static const char* tasks[];

	int currentTask;
	int showAxes;

	DebugDraw* mUpAxis;
	DebugDraw* mRightAxis;
	DebugDraw* mForwardAxis;
	
	Transform objectTransform;

	// for task1
	Pose pose;
	DebugDraw* poseHelper;

	// for task 2
	DebugDraw* skeletonHelper;

	// for task 3
	DebugDraw* restPoseHelper;
	DebugDraw* bindPoseHelper;
	Skeleton skeleton;

	// for task 4
	std::vector<Mesh> meshes;
	Shader* shader;
    Shader* shader_skin;
	Texture* diffuseTexture = NULL;

	Camera* camera;
	bool dragging = false;
	bool moving = false;

public:

    int applyBindPose = 0;

	void init();
	void update(float inDeltaTime);
	void render(float inAspectRatio);
	void ImGui(nk_context* context);
	void shutdown();

	// initialize task functions
	void initPose();
	void initSkeleton();
	void initSkeletonFromGLTF(cgltf_data* data);
	void initMeshesFromGLTF(cgltf_data* data);

	// input event callbacks
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
