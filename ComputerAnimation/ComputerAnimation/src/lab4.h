#pragma once

#include "application.h"
#include "camera.h"
#include <vector>
#include "external/cgltf.h"
#include "shading/debugDraw.h"
#include "shading/mesh.h"
#include "shading/texture.h"
#include "animation/pose.h"

#include "animation/ikSolver.h"
#include "animation/CCDSolver.h"
#include "animation/FABRIKSolver.h"

class Lab4 : public Application {
	struct AnimationInstance {
		Pose animatedPose;
		std::vector <mat4> posePalette;
		Transform model;
	};
protected:
	enum tasks { TASK1, TASK2, TASK3 };
	static const char* tasks[];

	int currentTask;
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

	std::vector<Mesh> meshes;
	Skeleton skeleton;
	Texture* tex;

	Transform objectTransform;

	// for IK
	enum solvers { CCD, FABRIK };
	static const char* solvers[];
	int currentSolver;
	
	IKSolver* currSolver;
	CCDSolver CCDSolver;
	FABRIKSolver FABRIKSolver;

	Transform target;
	
	AnimationInstance IKInfo;
	DebugDraw* CCDchainLines;
	DebugDraw* CCDchainPoints;
	DebugDraw* FABRIKchainLines;
	DebugDraw* FABRIKchainPoints;
	DebugDraw* targetVisual[3];

public:
	void init();
	
	void createChain();
	void createChainFromCharacter();

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
