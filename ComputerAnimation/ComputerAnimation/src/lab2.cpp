#include <GLFW/glfw3.h>
#include <windows.h>
#include <iostream>

#include "lab2.h"
#include "loaders/gLTFLoader.h"
#include "shading/uniform.h"

const char* Lab2::tasks[] = {"Pose", "Skeleton", "Load skeleton poses", "CPU skinning", "GPU skinning" };

void Lab2::init() {

	// Init here the attributes
	// ...
	
	// axes
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

	camera = new Camera();
	camera->setPerspective(60.0f, 1, 0.01f, 1000.0f);
	camera->lookAt(vec3(0, 1, 10), vec3(0, 1, 0), vec3(0, 1, 0));

	// for TASK 1
	initPose();
	
	// for TASK 2: 
	initSkeleton();

	// for TASK 3 and TASK 4
	bindPoseHelper = new DebugDraw();
	restPoseHelper = new DebugDraw();
	cgltf_data* gltf = loadGLTFFile("assets/Woman.gltf"); // parse the data of the specified file path
	
	// for TASK 3
	initSkeletonFromGLTF(gltf);
	
	// for TASK 4
	initMeshesFromGLTF(gltf);
	
	freeGLTFFile(gltf);

	// load shaders to render the meshes
	shader = new Shader("shaders/shader.vs", "shaders/shader.fs");

	currentTask = TASK1;
}

// TASK 1
void Lab2::initPose() {
	// Example: Create manually a pose of 6 joints: spine (1) <--| hip (0) |--> left upper leg (2)  |--> left leg (4)
	//																	   |--> right upper leg (3) |--> right leg (5)

	// TO DO: 
	// Initialize the array of joints of the pose using the number of joints you will create (example: 6)
	pose = Pose(6);

	// Initialize the configuration of the joints
	// For all joints of your pose:
	//		1. Create the local transform of the joint and assign the local position (relative to its parent)
	//		2. Set the transform to the joint array of the pose
	//		3. Set the parent of the joint using the joints array indices. If the joint doesn't have parent (root joint), set -1 as id

	//// hip
	Transform hipTransform;
	hipTransform.position = vec3(0.0f, 0.0f, 0.0f); //center
	pose.setLocalTransform(0, hipTransform);
	pose.setParent(0, -1);  //no parent

	// spine
	Transform spineTransform;
	spineTransform.position = vec3(0.0f, 1.0f, 0.0f); //above hip
	pose.setLocalTransform(1, spineTransform);
	pose.setParent(1, 0);  // hip child

	// left upper leg
	Transform leftUpperLegTransform;
	leftUpperLegTransform.position = vec3(-1.0f, 0.0f, 0.0f); //left of hip
	pose.setLocalTransform(2, leftUpperLegTransform);
	pose.setParent(2, 0);  // hip child

	// right upper leg
	Transform rightUpperLegTransform;
	rightUpperLegTransform.position = vec3(1.0f, 0.0f, 0.0f); //right of hip
	pose.setLocalTransform(3, rightUpperLegTransform);
	pose.setParent(3, 0);  // hip child

	// left leg
	Transform leftLegTransform;
	leftLegTransform.position = vec3(0.0f, -1.0f, 0.0f); //down of left upper leg
	pose.setLocalTransform(4, leftLegTransform);
	pose.setParent(4, 2);  // left upper leg child

	// right leg
	Transform rightLegTransform;
	rightLegTransform.position = vec3(0.0f, -1.0f, 0.0f); //down of right upper leg
	pose.setLocalTransform(5, rightLegTransform);
	pose.setParent(5, 3);  // right upper leg child


	// pose helper drawing
	poseHelper = new DebugDraw();
	// For each joint a line (bone) has to be draw from the joint world (global) position to the parent's joint world position
	for (unsigned int i = 0; i < pose.size(); i++) 
	{
		// TO DO:
		// Get the world position of the joint and push it to the poseHelper
		vec3 jointWorldPos = pose.getGlobalTransform(i).position;
		poseHelper->push(jointWorldPos);

		// If the joint has a parent (index >= 0), get the world position of the parent and push it to the poseHelper
		int jointParent = pose.getParent(i);
		if (jointParent > 0)
		{
			vec3 parentWorldPos = pose.getGlobalTransform(jointParent).position;
			poseHelper->push(parentWorldPos);
		}
	}
	// TO DO: Uncomment
	poseHelper->updateOpenGLBuffers();
}

// TASK 2
void Lab2::initSkeleton() {

	// Create an instance of skeleton setting the created pose as the rest pose and the bind pose (assuming that are the same), and the names of the joints
	std::vector<std::string> jointNames = { "Hip", "Spine", "Left Upper Leg", "Right Upper Leg", "Left leg", "Right Leg"};
	Skeleton skeleton = Skeleton(pose, pose, jointNames);
	
	// pose helper drawing
	skeletonHelper = new DebugDraw();

	// Get the bind pose of the skeleton 
	Pose bindPose = skeleton.getBindPose();

	// For each joint a line (bone) has to be draw from the joint world (global) position to the parent's joint world position
	for (int i = 1; i < bindPose.size(); i++) 
	{
		// If the joint has a parent, the joint position has to be converted into a world position:
		// 1. Get the world matrix of the parent using the inverse bind pose of the parent
		// 2. Get the world matrix of the joint multiplying the local matrix of the joint by the inverse of the parent's world matrix
		// 3. Push the world position of the joint into the skeletonHelper

		mat4 parentWorldMatrix = skeleton.getInvBindPose()[bindPose.getParent(i)];
		mat4 jointWorldMatrix = transformToMat4(bindPose.getLocalTransform(i)) * parentWorldMatrix;
		vec3 jointWorldPos = vec3{ jointWorldMatrix.position.x,jointWorldMatrix.position.y,jointWorldMatrix.position.z };
		skeletonHelper->push(jointWorldPos);

	// Do the same with its parent to get the final position of the bone

		int parentIndex = bindPose.getParent(i);
		if (parentIndex >= 0)
		{
			mat4 parentWorldMatrix2 = skeleton.getInvBindPose()[parentIndex];
			mat4 boneWorldMatrix = transformToMat4(bindPose.getLocalTransform(parentIndex)) * parentWorldMatrix2;
			vec3 parentWorldPos = vec3{ parentWorldMatrix2.position.x, parentWorldMatrix2.position.y, parentWorldMatrix2.position.z };
			skeletonHelper->push(parentWorldPos);
		}

	}
	// TO DO: Uncomment
	skeletonHelper->updateOpenGLBuffers();
}

// TASK 3
void Lab2::initSkeletonFromGLTF(cgltf_data* data) {

	// TO DO: initialize the skeleton attribute loading the skeleton of the gltf: use the loadSkeleton function
	skeleton = loadSkeleton(data);

	// Load the rest pose and the bind pose from the skeleton loaded from the gltf
	Pose restPose = skeleton.getRestPose();
	Pose bindPose = skeleton.getBindPose();

	for (unsigned int i = 0; i < restPose.size(); i++) 
	{
		// Do the same as in initPose() function
		// Get the world position of the joint and push it to the poseHelper
		vec3 jointWorldPos = restPose.getGlobalTransform(i).position;
		restPoseHelper->push(jointWorldPos);
	
		// If the joint has a parent (index >= 0), get the world position of the parent and push it to the poseHelper
		int jointParent = restPose.getParent(i);
		if (jointParent >= 0)
		{
			vec3 parentWorldPos = restPose.getGlobalTransform(jointParent).position;
			restPoseHelper->push(parentWorldPos);
		}
	}
	//TO DO: Uncomment
	restPoseHelper->updateOpenGLBuffers();

	for (unsigned int i = 0; i < bindPose.size(); i++) 
	{
		// Do the same as in initPose() function
		// Get the world position of the joint and push it to the poseHelper
		vec3 jointWorldPos = bindPose.getGlobalTransform(i).position;
		bindPoseHelper->push(jointWorldPos);
	
		// If the joint has a parent (index >= 0), get the world position of the parent and push it to the poseHelper
		int jointParent = bindPose.getParent(i);
		if (jointParent >= 0)
		{
			vec3 parentWorldPos = bindPose.getGlobalTransform(jointParent).position;
			bindPoseHelper->push(parentWorldPos);
		}
	}
	//TO DO: Uncomment
	bindPoseHelper->updateOpenGLBuffers();
}

// TASK 4
void Lab2::initMeshesFromGLTF(cgltf_data* data) {
	// TO DO: initialize the vector of meshes with the loaded meshes from the gltf file: use the loadeMeshes function
	//meshes = ...;
	// 
}

void Lab2::render(float inAspectRatio) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glClearColor(0.19f, 0.19f, 0.19f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glPointSize(5.0f);
	
	camera->setPerspective(60.0f, inAspectRatio, 0.01f, 1000.0f);
	mat4 view_projection = camera->getViewProjectionMatrix();
	mat4 model = transformToMat4(objectTransform);

	switch (currentTask) {
		case TASK1:
			poseHelper->draw(DebugDrawMode::Lines, vec3(1, 1, 1), view_projection * model);
			break;

		case TASK2:
			skeletonHelper->draw(DebugDrawMode::Lines, vec3(1, 0, 1), view_projection * model);
			break;

		case TASK3:
			restPoseHelper->draw(DebugDrawMode::Lines, vec3(1, 1, 1), view_projection); // rest pose
			bindPoseHelper->draw(DebugDrawMode::Lines, vec3(1, 0, 1), view_projection * model); // bind pose
			break;

		case TASK4:
			// CPU Skinned Mesh
			shader->Bind();
			// send the uniforms to the GPU to render the meshes
			Uniform<mat4>::Set(shader->GetUniform("model"), model);
			Uniform<mat4>::Set(shader->GetUniform("view"), camera->view_matrix);
			Uniform<mat4>::Set(shader->GetUniform("projection"), camera->projection_matrix);
			Uniform<vec3>::Set(shader->GetUniform("light"), vec3(1, 1, 1));
			//diffuseTexture->Set(shader->GetUniform("tex0"), 0);
			for (unsigned int i = 0, size = (unsigned int)meshes.size(); i < size; ++i) {
				meshes[i].bind(shader->GetAttribute("position"), shader->GetAttribute("normal"), shader->GetAttribute("texCoord"), -1, -1);
				meshes[i].draw();
				meshes[i].unBind(shader->GetAttribute("position"), shader->GetAttribute("normal"), shader->GetAttribute("texCoord"), -1, -1);
			}
			//diffuseTexture->UnSet(0);
			shader->UnBind();
			break;
	}
	
	if (showAxes) {
		mUpAxis->draw(DebugDrawMode::Lines, vec3(0, 1, 0), view_projection);
		mRightAxis->draw(DebugDrawMode::Lines, vec3(1, 0, 0), view_projection);
		mForwardAxis->draw(DebugDrawMode::Lines, vec3(0, 0, 1), view_projection);
	}	
}

void Lab2::update(float inDeltaTime) {

	// Perform the computations and update the attributes here
	// ...

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

	// TO DO: do the skinning (call CPUskin) for each mesh using the loaded skeleton and the rest pose (or the bind pose if the applyBindPose attribute is active) from the gltf file
	if (applyBindPose) {
		// For each mesh, apply the skinning using the bind pose of the skeleton
	}
	else {
		// For each mesh, apply the skinning using the rest pose of the skeleton
	}
}

// Example of using Nuklear GUI API: https://github.com/vurtun/nuklear/blob/master/example/extended.c
void Lab2::ImGui(nk_context* context) {

	static float range_float_min = -10;
	static float range_float_max = 10;

	char d[20];

	if (nk_begin(context, "Lab 2 Controls", nk_rect(5.0f, 70.0f, 300.0f, 300.0f), NK_WINDOW_MINIMIZABLE | NK_WINDOW_MOVABLE)) {
		nk_layout_row_static(context, 25, 200, 1);
		nk_layout_row_static(context, 25, 200, 1);
		nk_checkbox_label(context, "Show axes", &showAxes);

		currentTask = nk_combo(context, tasks, NK_LEN(tasks), currentTask, 25, nk_vec2(200, 200));

		switch (currentTask) {
			case TASK1: case TASK2:
				nk_layout_row_static(context, 25, 200, 1);
				nk_label(context, "Position", NK_TEXT_LEFT);
				nk_property_float(context, "#x", range_float_min, &objectTransform.position.x, range_float_max, 0.1, 0.1);
				nk_property_float(context, "#y", range_float_min, &objectTransform.position.y, range_float_max, 0.1, 0.1);
				nk_property_float(context, "#z", range_float_min, &objectTransform.position.z, range_float_max, 0.1, 0.1);

				nk_layout_row_static(context, 25, 200, 1);
				nk_label(context, "Rotation", NK_TEXT_LEFT);
				nk_property_float(context, "#x", range_float_min, &objectTransform.rotation.x, range_float_max, 0.1, 0.1);
				nk_property_float(context, "#y", range_float_min, &objectTransform.rotation.y, range_float_max, 0.1, 0.1);
				nk_property_float(context, "#z", range_float_min, &objectTransform.rotation.z, range_float_max, 0.1, 0.1);
				nk_property_float(context, "#w", range_float_min, &objectTransform.rotation.w, range_float_max, 0.1, 0.1);
				normalize(objectTransform.rotation);

				nk_layout_row_static(context, 25, 200, 1);
				nk_label(context, "Scale", NK_TEXT_LEFT);
				nk_property_float(context, "#x", range_float_min, &objectTransform.scale.x, range_float_max, 0.1, 0.1);
				nk_property_float(context, "#y", range_float_min, &objectTransform.scale.y, range_float_max, 0.1, 0.1);
				nk_property_float(context, "#z", range_float_min, &objectTransform.scale.z, range_float_max, 0.1, 0.1);
				break;

			case TASK3:
				// Bind pose
				nk_layout_row_static(context, 25, 200, 1);
				nk_label(context, "BIND POSE", NK_TEXT_CENTERED);
				nk_layout_row_static(context, 25, 200, 1);
				nk_label(context, "Position", NK_TEXT_LEFT);
				nk_property_float(context, "#x", range_float_min, &objectTransform.position.x, range_float_max, 0.1, 0.1);
				nk_property_float(context, "#y", range_float_min, &objectTransform.position.y, range_float_max, 0.1, 0.1);
				nk_property_float(context, "#z", range_float_min, &objectTransform.position.z, range_float_max, 0.1, 0.1);

				nk_layout_row_static(context, 25, 200, 1);
				nk_label(context, "Rotation", NK_TEXT_LEFT);
				nk_property_float(context, "#x", range_float_min, &objectTransform.rotation.x, range_float_max, 0.1, 0.1);
				nk_property_float(context, "#y", range_float_min, &objectTransform.rotation.y, range_float_max, 0.1, 0.1);
				nk_property_float(context, "#z", range_float_min, &objectTransform.rotation.z, range_float_max, 0.1, 0.1);
				nk_property_float(context, "#w", range_float_min, &objectTransform.rotation.w, range_float_max, 0.1, 0.1);
				normalize(objectTransform.rotation);

				nk_layout_row_static(context, 25, 200, 1);
				nk_label(context, "Scale", NK_TEXT_LEFT);
				nk_property_float(context, "#x", range_float_min, &objectTransform.scale.x, range_float_max, 0.1, 0.1);
				nk_property_float(context, "#y", range_float_min, &objectTransform.scale.y, range_float_max, 0.1, 0.1);
				nk_property_float(context, "#z", range_float_min, &objectTransform.scale.z, range_float_max, 0.1, 0.1);
				break;

			case TASK4:
				nk_layout_row_static(context, 25, 200, 1);
				nk_label(context, "SKINNING", NK_TEXT_CENTERED);
				nk_layout_row_static(context, 25, 200, 1);
				nk_checkbox_label(context, "Apply bind pose", &applyBindPose);
				nk_layout_row_static(context, 25, 200, 1);
				nk_label(context, "Position", NK_TEXT_LEFT);
				nk_property_float(context, "#x", range_float_min, &objectTransform.position.x, range_float_max, 0.1, 0.1);
				nk_property_float(context, "#y", range_float_min, &objectTransform.position.y, range_float_max, 0.1, 0.1);
				nk_property_float(context, "#z", range_float_min, &objectTransform.position.z, range_float_max, 0.1, 0.1);

				nk_layout_row_static(context, 25, 200, 1);
				nk_label(context, "Rotation", NK_TEXT_LEFT);
				nk_property_float(context, "#x", range_float_min, &objectTransform.rotation.x, range_float_max, 0.1, 0.1);
				nk_property_float(context, "#y", range_float_min, &objectTransform.rotation.y, range_float_max, 0.1, 0.1);
				nk_property_float(context, "#z", range_float_min, &objectTransform.rotation.z, range_float_max, 0.1, 0.1);
				nk_property_float(context, "#w", range_float_min, &objectTransform.rotation.w, range_float_max, 0.1, 0.1);
				normalize(objectTransform.rotation);

				nk_layout_row_static(context, 25, 200, 1);
				nk_label(context, "Scale", NK_TEXT_LEFT);
				nk_property_float(context, "#x", range_float_min, &objectTransform.scale.x, range_float_max, 0.1, 0.1);
				nk_property_float(context, "#y", range_float_min, &objectTransform.scale.y, range_float_max, 0.1, 0.1);
				nk_property_float(context, "#z", range_float_min, &objectTransform.scale.z, range_float_max, 0.1, 0.1);
				break;
		}
	}

	nk_end(context);
}

void Lab2::shutdown() { 
	delete mUpAxis;
	delete mRightAxis;
	delete mForwardAxis;
	delete poseHelper;
	delete skeletonHelper;
	delete restPoseHelper;
	delete bindPoseHelper;
	delete shader;
}


void Lab2::onKeyDown(int key, int scancode) { 
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

void Lab2::onKeyUp(int key, int scancode) { 
	// keycodes: https://www.glfw.org/docs/3.3/group__keys.html
	switch (key) {
		case GLFW_KEY_T:
			std::cout << "T released" << std::endl;
			break;
	}
};

void Lab2::onRightMouseButtonDown() {
	//std::cout << "Mouse position: " << std::endl;
	//std::cout << "x: " << mousePosition.x << ", y: " << mousePosition.y << std::endl;
	dragging = true;
	lastMousePosition = mousePosition;
};

void Lab2::onRightMouseButtonUp() {
	dragging = false;
	lastMousePosition = mousePosition;
};

void Lab2::onLeftMouseButtonDown() { };

void Lab2::onLeftMouseButtonUp() { };

void Lab2::onMiddleMouseButtonDown() {
	moving = true;
};
void Lab2::onMiddleMouseButtonUp() {
	moving = false;
};

void Lab2::onScroll(double xOffset, double yOffset) {
	//std::cout << xOffset << std::endl;
	//std::cout << yOffset << std::endl;
};
