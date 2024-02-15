#include <GLFW/glfw3.h>
#include <windows.h>
#include <iostream>
#include <math.h>

#include "lab3.h"
#include "../loaders/gLTFLoader.h"
#include "../shading/uniform.h"
#include "../math/quat.h"
#include "animation/blending.h"
		
const char* Lab3::tasks[] = { "Tracks", "Interpolation", "Clip animation", "Additive", "Crossfade" };
const char* Lab3::interpolation[] = { "Step", "Linear", "Cubic" };

void Lab3::init() {

	// Debug helpers
	showAxes = true;
	showSkeleton = false;

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
	mForwardAxis->push(vec3(0, 0, -1));
	mForwardAxis->updateOpenGLBuffers();

	poseHelper = new DebugDraw();

	// Init camera
	camera = new Camera();
	camera->setPerspective(60.0f, 1, 0.01f, 1000.0f);
	camera->lookAt(vec3(0, 3, 10), vec3(0, 1, 0), vec3(0, 1, 0));

	// Load data from GLTF
	cgltf_data* gltf = loadGLTFFile("assets/Woman.gltf"); // parse the data of the specified file path
	tex = new Texture("assets/Woman.png");
	meshes = loadMeshes(gltf);
	skeleton = loadSkeleton(gltf);
	clips = loadAnimationClips(gltf);

	freeGLTFFile(gltf);

	// Load shaders to render the meshes
	shader = new Shader("shaders/skinned.vs", "shaders/texture.fs");

	for (unsigned int i = 0, size = (unsigned int)meshes.size(); i < size; ++i) {
		meshes[i].updateOpenGLBuffers();
	}

	animInfo.animatedPose = skeleton.getRestPose();
	animInfo.poseMatrices.resize(skeleton.getRestPose().size());
	animInfo.poseMatrices = animInfo.animatedPose.getGlobalMatrices();
	// Setup initial state for task 1
	animInfo.model.position = vec3(-2, 0, 0);
	animInfo.model.rotation = quat(0, 0.707, 0, 0.707);

	additiveIndex = 0;
	for (unsigned int i = 0, size = (unsigned int)clips.size(); i < size; ++i) {
		if (clips[i].getName() == "Lean_Left") {
			additiveIndex = i;
		}
		if (clips[i].getName() == "Running") {
			animInfo.clip = i;
		}
	}
	
	// TASK 1
	interpolationType = 1; // linear
	// [CA] To do: Init a track with minimum 3 tracks
	track.resize(3);
	track[0] = makeVectorFrame(0.0f, vec3(0.0f), vec3(-2.0f, 0.0f, 0.0f), vec3(0.0f));
	track[1] = makeVectorFrame(1.0f, vec3(0.0f), vec3(2.0f, 2.0f, 0.0f), vec3(0.0f)); 
	track[2] = makeVectorFrame(2.0f, vec3(0.0f), vec3(4.0f, 0.0f, 0.0f), vec3(0.0f)); 

	currentTime = 0.0f;
	selectedFrame = 0;
	
	// TASK 3:
	// Get the rotation track of the joint that you want to apply the animation. Assign this rotation track to a refrence QuaternionTrack variable
	QuaternionTrack &qt_la = clip[13].getRotationTrack(); // idx 13 corresponds to left arm joint
	// [CA] To do: Resize the track with the number of frames and assign the type of interpolation
	qt_la.resize(3); //5
	// [CA] To do: Create the quaternion frames
	qt_la[0] = makeQuaternionFrame(0.0f, quat(0.0f, 0.0f, 0.0f, 1.0f), quat(0.0f, 0.0f, 0.707f, 0.707f), quat(0.0f, 0.0f, 0.0f, 1.0f));
	qt_la[1] = makeQuaternionFrame(1.0f, quat(0.0f, 0.0f, 0.0f, 1.0f), quat(0.0f, 0.0f, 0.0f, -1.0f), quat(0.0f, 0.0f, 0.0f, 1.0f));
	qt_la[2] = makeQuaternionFrame(2.0f, quat(0.0f, 0.0f, 0.0f, 1.0f), quat(0.0f, 0.0f, 0.707f, 0.707f), quat(0.0f, 0.0f, 0.0f, 1.0f));
	//qt_la[3] = makeQuaternionFrame(3.0f, quat(0.0f, 0.0f, 0.0f, 1.0f), quat(0.0f, 0.0f, 0.0f, -1.0f), quat(0.0f, 0.0f, 0.0f, 1.0f));
	//qt_la[4] = makeQuaternionFrame(4.0f, quat(0.0f, 0.0f, 0.0f, 1.0f), quat(0.0f, 0.0f, 0.707f, 0.707f), quat(0.0f, 0.0f, 0.0f, 1.0f));
	// [CA] To do: Assign the transformation track to the track of the clip for the specific joint
	clip[13].getRotationTrack() = qt_la;
	// [CA] To do: Recalculate the duration of the clip
	clip.recalculateDuration();
	
	QuaternionTrack& qt_ra = clip[24].getRotationTrack(); // idx 24 corresponds to right fore-arm joint
	// [CA] To do: Resize the track with the number of frames and assign the type of interpolation
	qt_ra.resize(3);
	// [CA] To do: Create the quaternion frames
	qt_ra[0] = makeQuaternionFrame(0.0f, quat(0.0f, 0.0f, 0.0f, 1.0f), quat(0.0f, 0.0f, 0.0f, 0.0f), quat(0.0f, 0.0f, 0.0f, 1.0f));
	qt_ra[1] = makeQuaternionFrame(1.0f, quat(0.0f, 0.0f, 0.0f, 1.0f), quat(0.0f, 0.707f, -0.707f, 0.707f), quat(0.0f, 0.0f, 0.0f, 1.0f));
	qt_ra[2] = makeQuaternionFrame(2.0f, quat(0.0f, 0.0f, 0.0f, 1.0f), quat(0.0f, 0.0f, 0.0f, 0.0f), quat(0.0f, 0.0f, 0.0f, 1.0f));
	// [CA] To do: Assign the transformation track to the track of the clip for the specific joint
	clip[24].getRotationTrack() = qt_ra;
	// [CA] To do: Recalculate the duration of the clip
	clip.recalculateDuration();

	// TASK 4:
	// [CA] To do: Use the makeAdditivePose() method from the blending script
	additiveBase = makeAdditivePose(skeleton, clips[additiveIndex]);
	clips[additiveIndex].setLooping(false);
	additiveTime = 0.0f;
	addPose = skeleton.getRestPose();

	// Set current task
	currentTask = TASK3;
}

VectorFrame Lab3::makeVectorFrame(float time, const vec3& in, const vec3& value, const vec3& out) {
	VectorFrame result;
	result.time = time;
	result.in[0] = in.x;
	result.in[1] = in.y;
	result.in[2] = in.z;
	result.value[0] = value.x;
	result.value[1] = value.y;
	result.value[2] = value.z;
	result.out[0] = out.x;
	result.out[1] = out.y;
	result.out[2] = out.z;
	return result;
}

QuaternionFrame Lab3::makeQuaternionFrame(float time, const quat& in, const quat& value, const quat& out) {
	QuaternionFrame result;
	// [CA] To do: complete the function
	result.time = time;
	result.in[0] = in.x;
	result.in[1] = in.y;
	result.in[2] = in.z;
	result.in[3] = in.w;
	result.value[0] = value.x;
	result.value[1] = value.y;
	result.value[2] = value.z;
	result.value[3] = value.w;
	result.out[0] = out.x;
	result.out[1] = out.y;
	result.out[2] = out.z;
	result.out[3] = out.w;
	return result;
}

void Lab3::render(float inAspectRatio) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glClearColor(0.19f, 0.19f, 0.19f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glPointSize(5.0f);

	camera->setPerspective(60.0f, inAspectRatio, 0.01f, 1000.0f);
	mat4 view_projection = camera->getViewProjectionMatrix();
	mat4 model_aux = transformToMat4(objectTransform);
	mat4 model = transformToMat4(animInfo.model);

	switch (currentTask) {
		case TASK1: case TASK2: case TASK3: case TASK4:
		{
				// GPU Skinned Mesh
				shader->Bind();
				Uniform<mat4>::Set(shader->GetUniform("model"), model * model_aux);
				Uniform<mat4>::Set(shader->GetUniform("view_projection"), view_projection);
				Uniform<vec3>::Set(shader->GetUniform("light"), vec3(1, 1, 1));

				Uniform<mat4>::Set(shader->GetUniform("pose"), animInfo.poseMatrices);
				Uniform<mat4>::Set(shader->GetUniform("invBindPose"), skeleton.getInvBindPose());

				tex->Set(shader->GetUniform("tex0"), 0);
				for (unsigned int i = 0, size = (unsigned int)meshes.size(); i < size; ++i) {
					meshes[i].bind(shader->GetAttribute("position"), shader->GetAttribute("normal"), shader->GetAttribute("texCoord"), shader->GetAttribute("weights"), shader->GetAttribute("joints"));
					meshes[i].draw();
					meshes[i].unBind(shader->GetAttribute("position"), shader->GetAttribute("normal"), shader->GetAttribute("texCoord"), shader->GetAttribute("weights"), shader->GetAttribute("joints"));
				}
				tex->UnSet(0);
				shader->UnBind();
				break;
		}
	}

	if (showAxes) {
		mUpAxis->draw(DebugDrawMode::Lines, vec3(0, 1, 0), view_projection);
		mRightAxis->draw(DebugDrawMode::Lines, vec3(1, 0, 0), view_projection);
		mForwardAxis->draw(DebugDrawMode::Lines, vec3(0, 0, 1), view_projection);
	}
	if (showSkeleton) {
		glDisable(GL_DEPTH_TEST);
		poseHelper->updateOpenGLBuffers();
		poseHelper->draw(DebugDrawMode::Lines, vec3(0, 1, 1), view_projection * model * model_aux);
		glEnable(GL_DEPTH_TEST);
	}
}

void Lab3::update(float inDeltaTime) {

	switch (currentTask) {
		case TASK1: case TASK2:
		{
			// [CA] To do: Sample the given clip and update poseMatrices the animInfo
			animInfo.playback = clips[animInfo.clip].sample(animInfo.animatedPose, currentTime);
			animInfo.poseMatrices = animInfo.animatedPose.getGlobalMatrices();

			// [CA] To do: Update objectTransform with the track information
			for (unsigned int i = 0; i < meshes.size(); i++) {
				meshes[i].CPUSkin(skeleton, animInfo.animatedPose);
				meshes[i].updateOpenGLBuffers();
			}
			break;
		}
		 case TASK3:
		 {
			 // [CA] To do: Sample YOUR CLIP and update poseMatrices the animInfo
			 animInfo.playback = clip.sample(animInfo.animatedPose, currentTime);
			 animInfo.poseMatrices = animInfo.animatedPose.getGlobalMatrices();

			 // [CA] To do: Update objectTransform with the track information
			 for (unsigned int i = 0; i < meshes.size(); i++) {
				 meshes[i].CPUSkin(skeleton, animInfo.animatedPose);
				 meshes[i].updateOpenGLBuffers();
			 }
			break;
			}
		 case TASK4:
		 {
			 // [CA] To do: Sample YOUR CLIP
			 animInfo.playback = clips[animInfo.clip].sample(animInfo.animatedPose, currentTime);

			 // [CA] To do: Update the addPose using the additiveTime and apply additive blending (remember that our blend root index is -1)
			 addPose = clip.sample(animInfo.animatedPose, additiveTime);
			 add(animInfo.animatedPose, animInfo.animatedPose, addPose, additiveBase, -1);

			 // [CA] To do: Update poseMatrices the animInfo
			 animInfo.poseMatrices = animInfo.animatedPose.getGlobalMatrices();
			 break;
		 }
		default:
			break;
	}

	currentTime += inDeltaTime;

	poseHelper->fromPose(animInfo.animatedPose);

	// Mouse update
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
void Lab3::ImGui(nk_context* context) {
	if (nk_begin(context, "Lab 3 Controls", nk_rect(5.0f, 70.0f, 300.0f, 300.0f), NK_WINDOW_MINIMIZABLE | NK_WINDOW_MOVABLE)) {
		nk_layout_row_static(context, 25, 200, 1);
		nk_checkbox_label(context, "Show axes", &showAxes);
		nk_checkbox_label(context, "Show skeleton", &showSkeleton);
		currentTask = nk_combo(context, tasks, NK_LEN(tasks), currentTask, 25, nk_vec2(200, 200));
		nk_layout_row_static(context, 25, 200, 1);

		switch (currentTask) {
			case TASK1: case TASK2:
				interpolationType = nk_combo(context, interpolation, 3, interpolationType, 25, nk_vec2(200, 200));
				if (interpolationType == 0) track.setInterpolation(Interpolation::Constant);
				if (interpolationType == 1) track.setInterpolation(Interpolation::Linear);
				if (interpolationType == 2)
				{
					track.setInterpolation(Interpolation::Cubic);
					
					const char* frameIndex[]  = { "1", "2", "3", "4", "5"}; // [CA] To do: update with the n� of frames of the track you created
					nk_layout_row_static(context, 25, 200, 1);
					nk_label(context, "Edit track slopes", NK_TEXT_CENTERED);
					nk_layout_row_static(context, 25, 200, 1);
					nk_label(context, "Frame", NK_TEXT_CENTERED);
					selectedFrame = nk_combo(context, frameIndex, track.size(), selectedFrame, 25, nk_vec2(200, 200));
					nk_layout_row_static(context, 25, 200, 1);
					nk_label(context, "Slope in", NK_TEXT_CENTERED);
					nk_property_float(context, "#in.x", -10, &track[selectedFrame].in[0], 10, 0.1, 0.1);
					nk_property_float(context, "#in.y", -10, &track[selectedFrame].in[1], 10, 0.1, 0.1);
					nk_property_float(context, "#in.z", -10, &track[selectedFrame].in[2], 10, 0.1, 0.1);
					nk_layout_row_static(context, 25, 200, 1);
					nk_label(context, "Slope out", NK_TEXT_CENTERED);
					nk_property_float(context, "#out.x", -10, &track[selectedFrame].out[0], 10, 0.1, 0.1);
					nk_property_float(context, "#out.y", -10, &track[selectedFrame].out[1], 10, 0.1, 0.1);
					nk_property_float(context, "#out.z", -10, &track[selectedFrame].out[2], 10, 0.1, 0.1);
				}
				break;
			case TASK3:
				interpolationType = nk_combo(context, interpolation, 3, interpolationType, 25, nk_vec2(200, 200));
				if (interpolationType == 0)
				{
					clip[13].getRotationTrack().setInterpolation(Interpolation::Constant);
					clip[24].getRotationTrack().setInterpolation(Interpolation::Constant);
				}
				if (interpolationType == 1)
				{
					clip[13].getRotationTrack().setInterpolation(Interpolation::Linear);
					clip[24].getRotationTrack().setInterpolation(Interpolation::Linear);
				}
				if (interpolationType == 2)
				{
					clip[13].getRotationTrack().setInterpolation(Interpolation::Cubic);

					const char* frameIndex[] = { "1", "2", "3"}; // [CA] To do: update with the n� of frames of the track you created
					nk_layout_row_static(context, 25, 200, 1);
					nk_label(context, "Edit track slopes", NK_TEXT_CENTERED);
					nk_layout_row_static(context, 25, 200, 1);
					nk_label(context, "Frame", NK_TEXT_CENTERED);
					selectedFrame = nk_combo(context, frameIndex, clip[13].getRotationTrack().size(), selectedFrame, 25, nk_vec2(200, 200));
					nk_layout_row_static(context, 25, 200, 1);
					nk_label(context, "Slope in", NK_TEXT_CENTERED);
					nk_property_float(context, "#in.x", -10, &clip[13].getRotationTrack()[selectedFrame].in[0], 10, 0.1, 0.1);
					nk_property_float(context, "#in.y", -10, &clip[13].getRotationTrack()[selectedFrame].in[1], 10, 0.1, 0.1);
					nk_property_float(context, "#in.z", -10, &clip[13].getRotationTrack()[selectedFrame].in[2], 10, 0.1, 0.1);
					nk_layout_row_static(context, 25, 200, 1);
					nk_label(context, "Slope out", NK_TEXT_CENTERED);
					nk_property_float(context, "#out.x", -10, &clip[13].getRotationTrack()[selectedFrame].out[0], 10, 0.1, 0.1);
					nk_property_float(context, "#out.y", -10, &clip[13].getRotationTrack()[selectedFrame].out[1], 10, 0.1, 0.1);
					nk_property_float(context, "#out.z", -10, &clip[13].getRotationTrack()[selectedFrame].out[2], 10, 0.1, 0.1);

					clip[24].getRotationTrack().setInterpolation(Interpolation::Cubic);
					
					const char* frameIndex2[] = { "1", "2", "3"}; // [CA] To do: update with the n� of frames of the track you created
					nk_layout_row_static(context, 25, 200, 1);
					nk_label(context, "Edit track slopes", NK_TEXT_CENTERED);
					nk_layout_row_static(context, 25, 200, 1);
					nk_label(context, "Frame", NK_TEXT_CENTERED);
					selectedFrame = nk_combo(context, frameIndex2, clip[24].getRotationTrack().size(), selectedFrame, 25, nk_vec2(200, 200));
					nk_layout_row_static(context, 25, 200, 1);
					nk_label(context, "Slope in", NK_TEXT_CENTERED);
					nk_property_float(context, "#in.x", -10, &clip[24].getRotationTrack()[selectedFrame].in[0], 10, 0.1, 0.1);
					nk_property_float(context, "#in.y", -10, &clip[24].getRotationTrack()[selectedFrame].in[1], 10, 0.1, 0.1);
					nk_property_float(context, "#in.z", -10, &clip[24].getRotationTrack()[selectedFrame].in[2], 10, 0.1, 0.1);
					nk_layout_row_static(context, 25, 200, 1);
					nk_label(context, "Slope out", NK_TEXT_CENTERED);
					nk_property_float(context, "#out.x", -10, &clip[24].getRotationTrack()[selectedFrame].out[0], 10, 0.1, 0.1);
					nk_property_float(context, "#out.y", -10, &clip[24].getRotationTrack()[selectedFrame].out[1], 10, 0.1, 0.1);
					nk_property_float(context, "#out.z", -10, &clip[24].getRotationTrack()[selectedFrame].out[2], 10, 0.1, 0.1);
				}
				break;
			case TASK4:
				nk_layout_row_dynamic(context, 20, 1);
				nk_label(context, "Additive Blending Time", NK_TEXT_LEFT);
				nk_layout_row_dynamic(context, 20, 2);
				nk_slider_float(context, 0.0f, &additiveTime, clips[additiveIndex].getDuration(), 0.1f);
				break;
		}
	}
	nk_end(context);
}

void Lab3::shutdown() {
	delete mUpAxis;
	delete mRightAxis;
	delete mForwardAxis;
}

void Lab3::onKeyDown(int key, int scancode) {
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

void Lab3::onKeyUp(int key, int scancode) {
	// keycodes: https://www.glfw.org/docs/3.3/group__keys.html
	switch (key) {
	case GLFW_KEY_T:
		std::cout << "T released" << std::endl;
		break;
	}
};


void Lab3::onRightMouseButtonDown() { 
	dragging = true;
	lastMousePosition = mousePosition;

};

void Lab3::onRightMouseButtonUp() { 
	dragging = false;
	lastMousePosition = mousePosition;
};

void Lab3::onLeftMouseButtonDown() { };

void Lab3::onLeftMouseButtonUp() { };

void Lab3::onMiddleMouseButtonDown() {
	moving = true;
};

void Lab3::onMiddleMouseButtonUp() {
	moving = false;
};

void Lab3::onScroll(double xOffset, double yOffset) { };