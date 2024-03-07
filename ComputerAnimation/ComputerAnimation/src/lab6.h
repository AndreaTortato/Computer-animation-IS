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
#include "../animation/clip.h"
#include "../math/mat4.h"

class Lab6 : public Application {
	struct AnimationInstance {
		Pose animatedPose;
		std::vector <mat4> posePalette;
		unsigned int clip;
		float playback;
		float speed;

		inline AnimationInstance() : clip(0), playback(0.0f), speed(1.0f) { }
	};

	struct Entity {
		mat4 model;
		std::vector<Mesh> meshes;
		Skeleton skeleton;
		DebugDraw* skeletonHelper = NULL;
		Pose pose;
		std::vector<Clip> clips;
		std::vector<std::vector<std::string>> morphTargetNames; // Contains the name of each morph target for each mesh
		std::vector<std::vector<float>> morphTargetInfluences; // Contains the influence of each morph target for each mesh
		int headJointIdx = -1;
	};

    struct Emotion {
        std::string name;
        std::vector < std::vector<vec2> > morphTargetsMapInfluences; // The "x" of the vec2 represents the morph target index and the "y" containts its weight in a range of [0,1]
        float weight;
        vec2 position; // 2D position for VA
    };


protected:
	enum tasks { TASK1, TASK2, TASK3};
	static const char* tasks[];

	int currentTask;

	int showAxes;
	int showSkeleton;
	int showBindPose;

	DebugDraw* mUpAxis;
	DebugDraw* mRightAxis;
	DebugDraw* mForwardAxis;

	Camera* camera;
	bool dragging = false;
	bool moving = false;
    bool activeScroll = true;

	Shader* shader;
	
	// Source characters
	Entity entity;
	
	// For task 2
	//Emotions emotions;
	std::vector<Emotion> emotions;
    int useVA = 0;

    vec2 currentVA;
    std::vector<float> precomputedVAweights;

	// For task 3
	Transform gazeTarget;
	DebugDraw* targetVisual[3];


public:
	void init();

	void createFaceEmotions();
    void createGazeSolver();
	void precomputeVoronoi(const int gridSize, const std::vector<vec2>& points, std::vector<float>& values);
	void interpolateVoronoi(int gridSize, vec2 point, std::vector<float> values, std::vector<float>& weights);

	void render(float inAspectRatio);
	void update(float inDeltaTime);
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

