#include "lab5.h"

#include <GLFW/glfw3.h>
#include <windows.h>
#include <iostream>
#include <math.h>

#include "../loaders/bvhLoader.h"
#include "../loaders/gLTFLoader.h"
#include "../shading/uniform.h"
#include "../math/quat.h"

const char* Lab5::tasks[] = { "Animation", "Retargeting", "BVH" };

#define DEG2RAD 0.0174533f

void Lab5::init() {

	// Debug helpers
	showAxes = true;
	showSkeleton = false;
	showTarget = true;
	showBadTarget = true;
	showSource = true;

	mUpAxis = new DebugDraw();
	mUpAxis->push(vec3(0, 0, 0));
	mUpAxis->push(vec3(0, 1, 0));
	mUpAxis->updateOpenGLBuffers();

	mRightAxis = new DebugDraw();
	mRightAxis->push(vec3(0, 0, 0));
	mRightAxis->push(vec3(1, 0, 0));
	mRightAxis->updateOpenGLBuffers();

	mForwardAxis = new DebugDraw();
	mForwardAxis->push(vec3(0, 0, 0));
	mForwardAxis->push(vec3(0, 0, 1));
	mForwardAxis->updateOpenGLBuffers();

	sourceGLTF.skeletonHelper = new DebugDraw();
	target.skeletonHelper = new DebugDraw();
	sourceBVH.skeletonHelper = new DebugDraw();
	badSkeletonHelper = new DebugDraw();

	// Init camera
	camera = new Camera();
	camera->setPerspective(60.0f, 1, 0.01f, 1000.0f);
	camera->lookAt(vec3(0, 3, 10), vec3(0, 1, 0), vec3(0, 1, 0));

	// Load shaders to render the meshes
	shader = new Shader("shaders/skinned.vs", "shaders/simple.fs");

	// TASK 1: Load data from GLTF (rat)
	cgltf_data* gltf = loadGLTFFile("assets/Dancing.glb"); // parse the data of the specified file path
	// [CA] To do: Load the meshes, the skeleton and clips from the gltf into the specific source entity 
	sourceGLTF.meshes = loadMeshes(gltf);
	sourceGLTF.skeleton = loadSkeleton(gltf);
	sourceGLTF.clips = loadAnimationClips(gltf);
	freeGLTFFile(gltf);

	for (unsigned int i = 0, size = (unsigned int)sourceGLTF.meshes.size(); i < size; ++i) {
		sourceGLTF.meshes[i].updateOpenGLBuffers();
	}

	// For the UI: Dynamically allocate memory for const char* array and populate
	numUIClips = (unsigned int)sourceGLTF.clips.size();
	std::vector<std::string> names;
	for (int i = 0; i < numUIClips; ++i) {
		names.push_back(sourceGLTF.clips[i].getName());
	}
	clips = new const char* [numUIClips];
	for (size_t i = 0; i < numUIClips; ++i) {
		clips[i] = strdup(names[i].c_str());
	}

	// mannequin
	gltf = loadGLTFFile("assets/Target.gltf"); // parse the data of the specified file path
	// [CA] To do: Load the meshes and the skeleton from the gltf into the target entity
	target.meshes = loadMeshes(gltf);
	target.skeleton = loadSkeleton(gltf);
	target.clips = sourceGLTF.clips;
	freeGLTFFile(gltf);

	for (unsigned int i = 0, size = (unsigned int)target.meshes.size(); i < size; ++i) {
		target.meshes[i].updateOpenGLBuffers();
	}

	// [CA] To do: Init the animationInfo instance with the source information (Tip: use the previous labs as reference)
	animationInfo.animatedPose = sourceGLTF.skeleton.getRestPose();
	animationInfo.posePalette.resize(sourceGLTF.skeleton.getRestPose().size());
	animationInfo.posePalette = animationInfo.animatedPose.getGlobalMatrices();

	// [CA] To do: Init the target poses with the target bind pose
	targetPose = target.skeleton.getBindPose();
	badTargetPose = target.skeleton.getBindPose();

	// TASK 2: Instance the retargeting solver with the skeletons for TASK 2
	retargetingSolver = new Retargeting(sourceGLTF.skeleton, target.skeleton, &animationInfo.animatedPose);
	
	// Set current task
	currentTask = TASK2;

	currentTime = 0.0f;

	// TASK 3: Load data from BVH
	// From cgspeed motion dataset https://sites.google.com/a/cgspeed.com/cgspeed/motion-capture/the-motionbuilder-friendly-bvh-conversion-release-of-cmus-motion-capture-database?authuser=0
	bvh::Bvh data = loadBVHFile("assets/Walk.bvh");
	// [CA] To do: Load the skeleton and the clip from the bvh into the specific source entity 
	sourceBVH.skeleton = loadSkeleton(data);
	//sourceBVH.clips[0] = loadAnimationClip(data);	
}

void Lab5::render(float inAspectRatio) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glClearColor(0.19f, 0.19f, 0.19f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glPointSize(5.0f);

	camera->setPerspective(camera->fov, inAspectRatio, 0.01f, 1000.0f);
	mat4 view_projection = camera->getViewProjectionMatrix();
	std::vector<mat4> poseMatrices;
	
	if (currentTask == TASK3) {
		sourceBVH.skeletonHelper->fromPose(animationInfo.animatedPose);
		sourceBVH.skeletonHelper->updateOpenGLBuffers();
		sourceBVH.skeletonHelper->draw(DebugDrawMode::Lines, vec3(0, 1, 1), view_projection * sourceBVH.model);
		sourceBVH.skeletonHelper->draw(DebugDrawMode::Points, vec3(0, 1, 1), view_projection * sourceBVH.model);
	}
		
	// Source mesh
	shader->Bind();
	Uniform<mat4>::Set(shader->GetUniform("view_projection"), view_projection);
	Uniform<vec3>::Set(shader->GetUniform("light"), vec3(1, 1, 1));

	if (currentTask != TASK3) 
	{
		Uniform<mat4>::Set(shader->GetUniform("model"), sourceGLTF.model);

		poseMatrices = animationInfo.posePalette;
		Uniform<mat4>::Set(shader->GetUniform("pose"), poseMatrices);
		Uniform<mat4>::Set(shader->GetUniform("invBindPose"), sourceGLTF.skeleton.getInvBindPose());

		if (showSource)
		{
			for (unsigned int i = 0, size = (unsigned int)sourceGLTF.meshes.size(); i < size; ++i) {
				sourceGLTF.meshes[i].bind(shader->GetAttribute("position"), shader->GetAttribute("normal"), -1, shader->GetAttribute("weights"), shader->GetAttribute("joints"));
				sourceGLTF.meshes[i].draw();
				sourceGLTF.meshes[i].unBind(shader->GetAttribute("position"), shader->GetAttribute("normal"), -1, shader->GetAttribute("weights"), shader->GetAttribute("joints"));
			}
		}
	}

	// Target mesh
	mat4 model_aux1;
	model_aux1.position.x = +2;
	if (showBindPose) {
		poseMatrices = target.skeleton.getBindPose().getGlobalMatrices();
	}
	else {
		poseMatrices = targetPose.getGlobalMatrices();
	}

		Uniform<mat4>::Set(shader->GetUniform("model"), model_aux1);
		Uniform<mat4>::Set(shader->GetUniform("pose"), poseMatrices);
		Uniform<mat4>::Set(shader->GetUniform("invBindPose"), target.skeleton.getInvBindPose());
	if (showTarget)
	{
		for (unsigned int i = 0, size = (unsigned int)target.meshes.size(); i < size; ++i) {
			target.meshes[i].bind(shader->GetAttribute("position"), shader->GetAttribute("normal"), -1, shader->GetAttribute("weights"), shader->GetAttribute("joints"));
			target.meshes[i].draw();
			target.meshes[i].unBind(shader->GetAttribute("position"), shader->GetAttribute("normal"), -1, shader->GetAttribute("weights"), shader->GetAttribute("joints"));
		}
	}

	// BAD Target mesh
	mat4 model_aux2;
	model_aux2.position.x = -2;
	if(showBindPose) {
		poseMatrices = target.skeleton.getBindPose().getGlobalMatrices();
	}
	else {
		poseMatrices = badTargetPose.getGlobalMatrices();
	}

	Uniform<mat4>::Set(shader->GetUniform("model"), model_aux2);
	Uniform<mat4>::Set(shader->GetUniform("pose"), poseMatrices);
	Uniform<mat4>::Set(shader->GetUniform("invBindPose"), target.skeleton.getInvBindPose());
	if (showBadTarget)
	{
		for (unsigned int i = 0, size = (unsigned int)target.meshes.size(); i < size; ++i) {
			target.meshes[i].bind(shader->GetAttribute("position"), shader->GetAttribute("normal"), -1, shader->GetAttribute("weights"), shader->GetAttribute("joints"));
			target.meshes[i].draw();
			target.meshes[i].unBind(shader->GetAttribute("position"), shader->GetAttribute("normal"), -1, shader->GetAttribute("weights"), shader->GetAttribute("joints"));
		}
	}

	shader->UnBind();

	if (showSkeleton) {
		glDisable(GL_DEPTH_TEST);

		if (currentTask != TASK3) {
			sourceGLTF.skeletonHelper->fromPose(animationInfo.animatedPose);
			sourceGLTF.skeletonHelper->updateOpenGLBuffers();
			sourceGLTF.skeletonHelper->draw(DebugDrawMode::Lines, vec3(0, 1, 1), view_projection * sourceGLTF.model);
			sourceGLTF.skeletonHelper->draw(DebugDrawMode::Points, vec3(0, 1, 1), view_projection * sourceGLTF.model);
		}

		if (showBindPose) {
			target.skeletonHelper->fromPose(target.skeleton.getBindPose());
			target.skeletonHelper->updateOpenGLBuffers();
			target.skeletonHelper->draw(DebugDrawMode::Lines, vec3(1, 0, 1), view_projection * model_aux1);
			target.skeletonHelper->draw(DebugDrawMode::Points, vec3(1, 0, 1), view_projection * model_aux1);

			badSkeletonHelper->fromPose(target.skeleton.getBindPose());
			badSkeletonHelper->updateOpenGLBuffers();
			badSkeletonHelper->draw(DebugDrawMode::Lines, vec3(1, 0, 1), view_projection * model_aux2);
			badSkeletonHelper->draw(DebugDrawMode::Points, vec3(1, 0, 1), view_projection * model_aux2);
		}
		else {
			target.skeletonHelper->fromPose(targetPose);
			target.skeletonHelper->updateOpenGLBuffers();
			target.skeletonHelper->draw(DebugDrawMode::Lines, vec3(0, 1, 1), view_projection * model_aux1);
			target.skeletonHelper->draw(DebugDrawMode::Points, vec3(0, 1, 1), view_projection * model_aux1);

			badSkeletonHelper->fromPose(badTargetPose);
			badSkeletonHelper->updateOpenGLBuffers();
			badSkeletonHelper->draw(DebugDrawMode::Lines, vec3(0, 1, 1), view_projection * model_aux2);
			badSkeletonHelper->draw(DebugDrawMode::Points, vec3(0, 1, 1), view_projection * model_aux2);
		}
		glEnable(GL_DEPTH_TEST);
	}

	if (showAxes) {
		mUpAxis->draw(DebugDrawMode::Lines, vec3(0, 1, 0), view_projection);
		mRightAxis->draw(DebugDrawMode::Lines, vec3(1, 0, 0), view_projection);
		mForwardAxis->draw(DebugDrawMode::Lines, vec3(0, 0, 1), view_projection);
	}
}

void Lab5::update(float inDeltaTime) {

	switch (currentTask) {

		targetPose = target.skeleton.getBindPose();

		case TASK1:
		{

			if (!showBindPose) {
				// [CA] To do: Basic sample src (gLTF) clip
				animationInfo.playback = sourceGLTF.clips[animationInfo.clip].sample(animationInfo.animatedPose, currentTime);			

				// [CA] To do: Sample the (bad) target pose without retargeting		
				animationInfo.playback = target.clips[animationInfo.clip].sample(badTargetPose, currentTime);

				animationInfo.posePalette = animationInfo.animatedPose.getGlobalMatrices();

			}		
			else
			{
				// Set the current pose as the src bind pose
				animationInfo.animatedPose = sourceGLTF.skeleton.getBindPose();
			}

			break;
		}
		case TASK2:
		{
			if (!showBindPose) {
				// [CA] To do: Basic sample src (gLTF) clip
				animationInfo.playback = sourceGLTF.clips[animationInfo.clip].sample(animationInfo.animatedPose, currentTime);

				// [CA] To do: Sample the (bad) target pose without retargeting
				animationInfo.playback = target.clips[animationInfo.clip].sample(badTargetPose, currentTime);

				// [CA] To do: Apply retargeting to the target pose
				retargetingSolver->solve(targetPose);
				animationInfo.playback = target.clips[animationInfo.clip].sample(targetPose, currentTime);

				animationInfo.posePalette = animationInfo.animatedPose.getGlobalMatrices();

			}
			else 
			{
				// Set the current pose as the src bind pose
				animationInfo.animatedPose = sourceGLTF.skeleton.getBindPose();

				// [CA] To do: Apply retargeting to the target pose
				retargetingSolver->solve(targetPose);
				animationInfo.playback = target.clips[animationInfo.clip].sample(targetPose, currentTime);

			}
			break;
		}
		case TASK3:
		{
			if (!showBindPose) {
				// [CA] To do: Basic sample src (BVH) clip
				animationInfo.playback = bvhClip.sample(animationInfo.animatedPose, currentTime);
				
				// [CA] To do: Sample the (bad) target pose without retargeting
				animationInfo.playback = target.clips[animationInfo.clip].sample(badTargetPose, currentTime);

				// [CA] To do: Apply retargeting to the target pose
				retargetingSolver->solve(targetPose);
				animationInfo.playback = target.clips[animationInfo.clip].sample(targetPose, currentTime);

				animationInfo.posePalette = animationInfo.animatedPose.getGlobalMatrices();
			}
			else 
			{
				// Set the current pose as the src bind pose

				animationInfo.animatedPose = sourceBVH.skeleton.getBindPose();
				// [CA] To do: Apply retargeting to the target pose
				retargetingSolver->solve(targetPose);
				animationInfo.playback = target.clips[animationInfo.clip].sample(targetPose, currentTime);
			}
			break;
		}
	}

	currentTime += inDeltaTime;

	// mouse update
	vec2 delta = lastMousePosition - mousePosition;
	if (dragging) {
		camera->orbit((float)delta.x * 0.01, vec3(0, 1, 0));
		camera->orbit((float)delta.y * 0.01, vec3(1, 0, 0));
	}
	if (moving) {
		camera->eye.x += delta.x * 0.01;
		camera->eye.y -= delta.y * 0.01;
		camera->center.x += delta.x * 0.01;
		camera->center.y -= delta.y * 0.01;
		camera->updateViewMatrix();
	}
	lastMousePosition = mousePosition;
}

// Example of using Nuklear GUI API: https://github.com/vurtun/nuklear/blob/master/example/extended.c
void Lab5::ImGui(nk_context* context) {

	if (nk_begin(context, "Lab 5 Controls", nk_rect(5.0f, 70.0f, 300.0f, 300.0f), NK_WINDOW_MINIMIZABLE | NK_WINDOW_MOVABLE)) {
		nk_layout_row_dynamic(context, 25, 1);
		nk_checkbox_label(context, "Show axes", &showAxes);
		nk_layout_row_dynamic(context, 25, 1);
		nk_checkbox_label(context, "Show skeletons", &showSkeleton);
		nk_layout_row_dynamic(context, 25, 1);
		nk_checkbox_label(context, "Apply bind pose", &showBindPose);
		nk_layout_row_dynamic(context, 25, 1);

		nk_layout_row_dynamic(context, 25, 1);
		nk_checkbox_label(context, "Source mesh", &showSource);
		nk_layout_row_dynamic(context, 25, 1);
		nk_checkbox_label(context, "Target mesh", &showTarget);
		nk_layout_row_dynamic(context, 25, 1);
		nk_checkbox_label(context, "Bad target mesh", &showBadTarget);

		nk_layout_row_dynamic(context, 25, 1);
		nk_label(context, "Tasks", NK_TEXT_CENTERED);
		int task = nk_combo(context, tasks, NK_LEN(tasks), currentTask, 25, nk_vec2(200, 200));
		if (task != currentTask) {
			currentTask = task;
			switch (currentTask) 
			{
				case TASK1: case TASK2:
					animationInfo.animatedPose = sourceGLTF.skeleton.getBindPose();
					// [CA] To do: Create the retargeting solver using the src gLTF skeleton
					retargetingSolver = new Retargeting(sourceGLTF.skeleton, target.skeleton, &animationInfo.animatedPose);
					break;
				case TASK3:
					animationInfo.animatedPose = sourceBVH.skeleton.getBindPose();
					animationInfo.clip = 0; // bvh only has 1 clip
					// [CA] To do: Create the retargeting solver using the src BVH skeleton
					retargetingSolver = new Retargeting(sourceGLTF.skeleton, target.skeleton, &animationInfo.animatedPose);
					break;
			}
		}

		if (currentTask != TASK3) {
			nk_layout_row_dynamic(context, 25, 1);
			nk_label(context, "Animations", NK_TEXT_CENTERED);
			animationInfo.clip = nk_combo(context, clips, numUIClips, animationInfo.clip, 25, nk_vec2(200, 200));
		}

		nk_layout_row_dynamic(context, 25, 1);
		nk_label(context, "Speed", NK_TEXT_CENTERED);
		nk_property_float(context, "#speed", 0, &animationInfo.speed, 3, 0.1, 0.1);
	}

	nk_end(context);
}

void Lab5::shutdown() {
	delete mUpAxis;
	delete mRightAxis;
	delete mForwardAxis;

	delete sourceGLTF.skeletonHelper;
	delete target.skeletonHelper;
	delete sourceBVH.skeletonHelper;

	delete retargetingSolver;
}

void Lab5::onKeyDown(int key, int scancode) {
	// keycodes: https://www.glfw.org/docs/3.3/group__keys.html
	switch (key) {
		case GLFW_KEY_ESCAPE: // quit
			close = true;
			break;
		case GLFW_KEY_T:
			std::cout << "T pressed" << std::endl;
			break;
	}
};

void Lab5::onKeyUp(int key, int scancode) {
	// keycodes: https://www.glfw.org/docs/3.3/group__keys.html
	switch (key) {
		case GLFW_KEY_T:
			std::cout << "T released" << std::endl;
			break;
	}
};

void Lab5::onRightMouseButtonDown() {
	dragging = true;
	lastMousePosition = mousePosition;
}

void Lab5::onRightMouseButtonUp() {
	dragging = false;
	lastMousePosition = mousePosition;
};

void Lab5::onLeftMouseButtonDown() { };

void Lab5::onLeftMouseButtonUp() { };

void Lab5::onMiddleMouseButtonDown() {
	moving = true;
};

void Lab5::onMiddleMouseButtonUp() {
	moving = false;
};

void Lab5::onScroll(double xOffset, double yOffset) {
	camera->fov -= yOffset * 5;
	if (camera->fov < 10)
		camera->fov = 10;
	else if (camera->fov > 150)
		camera->fov = 150;
};