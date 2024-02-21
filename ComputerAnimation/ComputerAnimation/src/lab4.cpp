
#include <GLFW/glfw3.h>
#include <windows.h>
#include <iostream>
#include <math.h>

#include "lab4.h"
#include "loaders/gLTFLoader.h"
#include "shading/uniform.h"
#include "math/quat.h"

const char* Lab4::tasks[] = { "Chains", "Character IK", "Constrains" };
const char* Lab4::solvers[] = { "CCD", "FABRIK" };
#define GIZMO_SIZE 0.25f
#define DEG2RAD 0.0174533f

void Lab4::init() {

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
	mForwardAxis->push(vec3(0, 0, 1));
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

	freeGLTFFile(gltf);

	for (unsigned int i = 0, size = (unsigned int)meshes.size(); i < size; ++i) {
		meshes[i].updateOpenGLBuffers();
	}

	// Load shaders to render the meshes
	shader = new Shader("shaders/skinned.vs", "shaders/texture.fs");

	IKInfo.animatedPose = skeleton.getRestPose();
	IKInfo.posePalette.resize(skeleton.getRestPose().size());
	IKInfo.posePalette = IKInfo.animatedPose.getGlobalMatrices();

	// Create chains
	createChain();
	// Set the initial target position
	target.position = vec3(2, 2, 0);
	// Set translation for FABRIK chain target gizmo
	objectTransform.position.x = 5;

	// Create CCD & FABRIK visual chains
	CCDchainLines = new DebugDraw();
	CCDchainPoints = new DebugDraw();
	FABRIKchainLines = new DebugDraw();
	FABRIKchainPoints = new DebugDraw();

	//Create visual target gizmo
	targetVisual[0] = new DebugDraw(2);
	targetVisual[1] = new DebugDraw(2);
	targetVisual[2] = new DebugDraw(2);

	// Set current task
	currentTask = TASK1;

	// Set current solver for character IK chain
	currentSolver = 0;
	currSolver = &CCDSolver;
}

void Lab4::createChain() {
	//TASK 1 & TASK 2
	// [CA] To do: Create a chain in local space and assign it to CCD and FABRIK solvers
	float segmentLength = 1.0f;
	int numJoints = 3;
	std::vector<Transform> chain;

	for (int i = 0; i < numJoints; ++i) {
		// Define the position of each joint along the x-axis (for simplicity)
		vec3 jointPosition(i * segmentLength, 0.0f, 0.0f);

		// Create a local transform for the joint
		Transform jointTransform;
		jointTransform.position = jointPosition;

		// Add the joint to the chain
		chain.push_back(jointTransform);
	}

	// Set the created chain to the CCD solve
	CCDSolver.setChain(chain);
	FABRIKSolver.setChain(chain);

	// Set an initial position to the target (for example, at the end of the chain)
	target.position = chain.back().position;
	
	// Solve the inverse kinematics problem using the CCD solver
	CCDSolver.solve(target);
	FABRIKSolver.solve(target);
}

void Lab4::createChainFromCharacter() {
	//TASK 3
	// [CA] To do: Create a chain using joints of the character and assign it to CCD and FABRIK solvers (the origin joint of the chain has to be in global space)
	
	std::vector<Transform> chain;

	for (int i = 0; i <= charachterjoint; ++i) 
	{
		Transform localTransform = skeleton.getRestPose().getLocalTransform(i);
		chain.push_back(localTransform);
	}
	
	CCDSolver.setChain(chain);
	FABRIKSolver.setChain(chain);
	
	// Set the target to the global position of the last joint of the chain
	target.position = chain.back().position;
	
	CCDSolver.solve(target);
	FABRIKSolver.solve(target);
}

void Lab4::render(float inAspectRatio) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glClearColor(0.19f, 0.19f, 0.19f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glPointSize(5.0f);

	camera->setPerspective(camera->fov, inAspectRatio, 0.01f, 1000.0f);
	mat4 view_projection = camera->getViewProjectionMatrix();
	mat4 model_aux = transformToMat4(objectTransform);
	mat4 model = transformToMat4(IKInfo.model);

	// Target gizmo visualization (for CCD chain and Character chain)
	(*targetVisual[0])[0] = target.position + vec3(GIZMO_SIZE, 0, 0);
	(*targetVisual[1])[0] = target.position + vec3(0, GIZMO_SIZE, 0);
	(*targetVisual[2])[0] = target.position + vec3(0, 0, GIZMO_SIZE);
	(*targetVisual[0])[1] = target.position - vec3(GIZMO_SIZE, 0, 0);
	(*targetVisual[1])[1] = target.position - vec3(0, GIZMO_SIZE, 0);
	(*targetVisual[2])[1] = target.position - vec3(0, 0, GIZMO_SIZE);
	targetVisual[0]->updateOpenGLBuffers();
	targetVisual[1]->updateOpenGLBuffers();
	targetVisual[2]->updateOpenGLBuffers();

	targetVisual[0]->draw(DebugDrawMode::Lines, vec3(1, 0, 0), view_projection);
	targetVisual[1]->draw(DebugDrawMode::Lines, vec3(0, 1, 0), view_projection);
	targetVisual[2]->draw(DebugDrawMode::Lines, vec3(0, 0, 1), view_projection);


	switch (currentTask) {
	case TASK1:
		// [CA] To do: Draw the chains using DebugDraw instances:

		//Draw CCD solver chain using linesFromIKSolver
		//1. Push joints to opengl (joints)
		CCDchainLines->linesFromIKSolver(CCDSolver);
		//2. Push bones to opengl (bones)
		CCDchainPoints->pointsFromIKSolver(CCDSolver);
		//3. Update open gl buffers
		CCDchainLines->updateOpenGLBuffers();
		CCDchainPoints->updateOpenGLBuffers();
		//4. Draw points and lines
		CCDchainLines->draw(DebugDrawMode::Lines, vec3(0, 1, 1), view_projection);
		CCDchainPoints->draw(DebugDrawMode::Points, vec3(0, 1, 1), view_projection);


		//Draw FABRIK solver chain
		//1. Push joints to opengl (joints)
		FABRIKchainLines->linesFromIKSolver(FABRIKSolver);
		//2. Push bones to opengl (bones)
		FABRIKchainPoints->pointsFromIKSolver(FABRIKSolver);
		//3. Update open gl buffers
		FABRIKchainLines->updateOpenGLBuffers();
		FABRIKchainPoints->updateOpenGLBuffers();
		//4. Draw points and lines
		FABRIKchainLines->draw(DebugDrawMode::Lines, vec3(1, 0, 1), view_projection);
		FABRIKchainPoints->draw(DebugDrawMode::Points, vec3(1, 0, 1), view_projection);


		// FABRIK chain target visualization
		targetVisual[0]->draw(DebugDrawMode::Lines, vec3(1, 0, 0), view_projection * model_aux);
		targetVisual[1]->draw(DebugDrawMode::Lines, vec3(0, 1, 0), view_projection * model_aux);
		targetVisual[2]->draw(DebugDrawMode::Lines, vec3(0, 0, 1), view_projection * model_aux);
		break;

	case TASK2:
	{
		// GPU Skinned Mesh
		shader->Bind();
		Uniform<mat4>::Set(shader->GetUniform("model"), model);
		Uniform<mat4>::Set(shader->GetUniform("view_projection"), view_projection);
		Uniform<vec3>::Set(shader->GetUniform("light"), vec3(1, 1, 1));

		std::vector<mat4> poseMatrices = IKInfo.animatedPose.getGlobalMatrices();
		Uniform<mat4>::Set(shader->GetUniform("pose"), poseMatrices);
		Uniform<mat4>::Set(shader->GetUniform("invBindPose"), skeleton.getInvBindPose());

		tex->Set(shader->GetUniform("tex0"), 0);
		for (unsigned int i = 0, size = (unsigned int)meshes.size(); i < size; ++i) {
			meshes[i].bind(shader->GetAttribute("position"), shader->GetAttribute("normal"), shader->GetAttribute("texCoord"), shader->GetAttribute("weights"), shader->GetAttribute("joints"));
			meshes[i].draw();
			meshes[i].unBind(shader->GetAttribute("position"), shader->GetAttribute("normal"), shader->GetAttribute("texCoord"), shader->GetAttribute("weights"), shader->GetAttribute("joints"));
		}
		tex->UnSet(0);
		shader->UnBind();

		if (showSkeleton) {
			glDisable(GL_DEPTH_TEST);
			poseHelper->updateOpenGLBuffers();
			if (currentSolver == 0)
			{
				poseHelper->draw(DebugDrawMode::Lines, vec3(0, 1, 1), view_projection * model);

			}
			if (currentSolver == 1)
			{
				poseHelper->draw(DebugDrawMode::Lines, vec3(1, 0, 1), view_projection * model);

			}
			glEnable(GL_DEPTH_TEST);
		}
		break;
	}
	}

	if (showAxes) {
		mUpAxis->draw(DebugDrawMode::Lines, vec3(0, 1, 0), view_projection);
		mRightAxis->draw(DebugDrawMode::Lines, vec3(1, 0, 0), view_projection);
		mForwardAxis->draw(DebugDrawMode::Lines, vec3(0, 0, 1), view_projection);
	}

}

void Lab4::update(float inDeltaTime) {


	switch (currentTask) {
	case TASK1:
		// [CA] To do:
		// Solve CCD and FABRIK IK for both chains
		
		CCDSolver.solve(target);
		FABRIKSolver.solve(target);

		break;
	case TASK2:
	{
		// [CA] To do:
		// Solve IK for character chain depending of the selected solver type

		if (currentSolver == 0)
		{
			CCDSolver.solve(target); 
			int chainSize = CCDSolver.getChain().size();

			// Update skeleton and current pose with the IK chain transforms
			// 1. Get origin joint in local space: Combine the inverse global transformation of its parent with its computed IK transformation
			Transform originLocalTransform = CCDSolver.getChain()[charachterjoint];

			// 2. Set the local transformation of the origin joint to the current pose
			IKInfo.animatedPose.setLocalTransform(charachterjoint, originLocalTransform);

			// 3. For the rest of the chain, set the local transformation of each joint into the corresponding current pose joint
			for (int i = chainSize - 1; i >= 0; --i)
			{
				Transform jointLocalTransform = CCDSolver.getChain()[i];
				IKInfo.animatedPose.setLocalTransform(i, jointLocalTransform);
			}
		}


		if (currentSolver == 1)
		{
			FABRIKSolver.solve(target);
			int chainSize = FABRIKSolver.getChain().size();

			// Update skeleton and current pose with the IK chain transforms
			// 1. Get origin joint in local space: Combine the inverse global transformation of its parent with its computed IK transformation
			Transform originLocalTransform = FABRIKSolver.getChain()[charachterjoint];

			// 2. Set the local transformation of the origin joint to the current pose
			IKInfo.animatedPose.setLocalTransform(charachterjoint, originLocalTransform);

			// 3. For the rest of the chain, set the local transformation of each joint into the corresponding current pose joint
			for (int i = chainSize - 1; i >= 0; --i)
			{
				Transform jointLocalTransform = FABRIKSolver.getChain()[i];
				IKInfo.animatedPose.setLocalTransform(i, jointLocalTransform);
			}
		}
		
		
		// Update the global matrices of the struct animaiton
		IKInfo.posePalette = IKInfo.animatedPose.getGlobalMatrices();
		
		// Update the poseHelper visualization with the current pose
		poseHelper->fromPose(IKInfo.animatedPose);
		break;
	}
	default:
		break;
	}

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
void Lab4::ImGui(nk_context* context) {
	if (nk_begin(context, "Lab 4 Controls", nk_rect(5.0f, 70.0f, 300.0f, 300.0f), NK_WINDOW_MINIMIZABLE | NK_WINDOW_MOVABLE)) {
		nk_layout_row_static(context, 25, 200, 1);
		nk_checkbox_label(context, "Show axes", &showAxes);

		int task = nk_combo(context, tasks, NK_LEN(tasks), currentTask, 25, nk_vec2(200, 200));
		if (task != currentTask) {
			currentTask = task;
			switch (currentTask) {
			case TASK1:
				// Create CCD & FABRIK chains 
				createChain();
				break;
			case TASK2:
				// Create IK chain from a character
				createChainFromCharacter();
				break;
			}
		}
		if (currentTask == TASK2) {
			nk_layout_row_static(context, 25, 200, 1);
			nk_checkbox_label(context, "Show skeleton", &showSkeleton);
			nk_layout_row_static(context, 25, 200, 1);
			int solver = nk_combo(context, solvers, NK_LEN(solvers), currentSolver, 25, nk_vec2(200, 200));
			if (solver != currentSolver) {
				currentSolver = solver;
				// [CA] To do: Change the current solver for the character IK chain
				//..

			}
		}

		nk_layout_row_static(context, 25, 200, 1);

		nk_label(context, "Target", NK_TEXT_CENTERED);
		nk_property_float(context, "#x", -10, &target.position.x, 10, 0.1, 0.1);
		nk_property_float(context, "#y", -10, &target.position.y, 10, 0.1, 0.1);
		nk_property_float(context, "#z", -10, &target.position.z, 10, 0.1, 0.1);
	}
	nk_end(context);
}

void Lab4::shutdown() {
	delete mUpAxis;
	delete mRightAxis;
	delete mForwardAxis;

	delete poseHelper;

	delete CCDchainLines;
	delete CCDchainPoints;
	delete FABRIKchainLines;
	delete FABRIKchainPoints;
}

void Lab4::onKeyDown(int key, int scancode) {
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

void Lab4::onKeyUp(int key, int scancode) {
	// keycodes: https://www.glfw.org/docs/3.3/group__keys.html
	switch (key) {
	case GLFW_KEY_T:
		std::cout << "T released" << std::endl;
		break;
	}
};


void Lab4::onRightMouseButtonDown() {
	dragging = true;
	lastMousePosition = mousePosition;

}

void Lab4::onRightMouseButtonUp() {
	dragging = false;
	lastMousePosition = mousePosition;
};

void Lab4::onLeftMouseButtonDown() { };

void Lab4::onLeftMouseButtonUp() { };

void Lab4::onMiddleMouseButtonDown() {
	moving = true;
};

void Lab4::onMiddleMouseButtonUp() {
	moving = false;
};

void Lab4::onScroll(double xOffset, double yOffset) {
	if (camera->fov > 20 && camera->fov < 100) {
		camera->fov += yOffset * 5;
	}
};