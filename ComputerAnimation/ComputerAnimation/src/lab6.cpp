#include "lab6.h"

#include <GLFW/glfw3.h>
#include <windows.h>
#include <iostream>
#include <math.h>

#include "../loaders/gLTFLoader.h"
#include "../shading/uniform.h"
#include "../external/nuklear_utils.h"

const char* Lab6::tasks[] = { "Morph Targets", "Expressions", "Gaze"};
#define GIZMO_SIZE 0.25f
#define GRID_SIZE 50

void Lab6::init() {

	// Debug helpers
	showAxes = false;
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

	entity.skeletonHelper = new DebugDraw();
	

	// Init camera
	camera = new Camera();
	camera->setPerspective(60.0f, 1, 0.001f, 1000.0f);
	camera->lookAt(vec3(0, 3, 10), vec3(0, 1, 0), vec3(0, 1, 0));

	// Load data from GLTF
	cgltf_data* gltf = loadGLTFFile("assets/Eva_Low.glb"); // parse the data of the specified file path
	entity.skeleton = loadSkeleton(gltf);
	entity.pose = entity.skeleton.getRestPose();
	entity.clips = loadAnimationClips(gltf);
	entity.meshes = loadMeshes(gltf);

	// [CA] To do: Initialize morph targets names and influences in entity using the meshes loaded data
	for (int i = 0; i < entity.meshes.size(); i++)
	{
		//get mesh of an entity
		Mesh m = entity.meshes[i];		
		//m.encodeMorphTargets();
	
		//get morph targets of a mesh
		std::vector<MorphTarget> mts = m.getMorphTargets();
		std::vector<std::string> mtNames;
		std::vector<float> mtInfluences;

		for (int j = 0; j < mts.size(); j++)
		{
			// name/influence of morphTarget j of mesh i:
			std::string name = mts[j].name;
			mtNames.push_back(name);
			mtInfluences.push_back(0.0);
		}
		entity.morphTargetNames.push_back(mtNames);
		entity.morphTargetInfluences.push_back(mtInfluences);
	}

	freeGLTFFile(gltf);
	
	// Load shaders to render the meshes
	shader = new Shader("shaders/morph.vs", "shaders/pbr.fs");
	
	// Set current task
	currentTask = TASK1;

	// For task 2
	createFaceEmotions();	

	// For task 3
	createGazeSolver();
	
}

void Lab6::createFaceEmotions() {

	// [CA] To do: Create the instances of the 6 basic emotions and push them into the emotions array
	Emotion emotion;

	emotion.name = "Neutral";
	emotion.position = vec2(0, 0);
	emotion.weight = 1;
	emotion.morphTargetsMapInfluences.resize(entity.morphTargetInfluences.size());
	emotions.push_back(emotion);
	
	emotion.name = "Happiness";
	emotion.position = vec2(0.5, 0.8);
	emotion.weight = 1;
	emotion.morphTargetsMapInfluences.resize(entity.morphTargetInfluences.size());
	emotions.push_back(emotion);

	emotion.name = "Sadness";
	emotion.position = vec2(-0.4, -0.2);
	emotion.weight = 1;
	emotion.morphTargetsMapInfluences.resize(entity.morphTargetInfluences.size());
	emotions.push_back(emotion);

	emotion.name = "Surprise";
	emotion.position = vec2(0, 0.7);
	emotion.weight = 1;
	emotion.morphTargetsMapInfluences.resize(entity.morphTargetInfluences.size());
	emotions.push_back(emotion);

	emotion.name = "Anger";
	emotion.position = vec2(-0.5, 0.8);
	emotion.weight = 1;
	emotion.morphTargetsMapInfluences.resize(entity.morphTargetInfluences.size());
	emotions.push_back(emotion);

	emotion.name = "Disgust";
	emotion.position = vec2(-0.2, 0.4);
	emotion.weight = 1;
	emotion.morphTargetsMapInfluences.resize(entity.morphTargetInfluences.size());
	emotions.push_back(emotion);

	emotion.name = "Fear";
	emotion.position = vec2(-0.3, 0.2);
	emotion.weight = 1;
	emotion.morphTargetsMapInfluences.resize(entity.morphTargetInfluences.size());
	emotions.push_back(emotion);

	// [CA] To do: Init the morphTargetsMapInfluences of each emotion with the index of the morph target and its influence 
	
	std::vector<std::vector<vec2>> &neutralMorphMapWeights = emotions[0].morphTargetsMapInfluences;
	std::vector<std::vector<vec2>> &happinessMorphMapWeights = emotions[1].morphTargetsMapInfluences;
	std::vector<std::vector<vec2>> &sadnessMorphMapWeights = emotions[2].morphTargetsMapInfluences;
	std::vector<std::vector<vec2>> &surpriseMorphMapWeights = emotions[3].morphTargetsMapInfluences;
	std::vector<std::vector<vec2>> &angerMorphMapWeights = emotions[4].morphTargetsMapInfluences;
	std::vector<std::vector<vec2>> &disgustMorphMapWeights = emotions[5].morphTargetsMapInfluences;
	std::vector<std::vector<vec2>> &fearMorphMapWeights = emotions[6].morphTargetsMapInfluences;

	for (unsigned int i = 0; i < entity.morphTargetNames.size(); i++) 
	{
		for (unsigned int j = 0; j < entity.morphTargetNames[i].size(); j++) 
		{
			neutralMorphMapWeights[i].push_back(vec2(j, emotions[0].weight));
			happinessMorphMapWeights[i].push_back(vec2(j, emotions[1].weight));
			sadnessMorphMapWeights[i].push_back(vec2(j, emotions[2].weight));
			surpriseMorphMapWeights[i].push_back(vec2(j, emotions[3].weight));
			angerMorphMapWeights[i].push_back(vec2(j, emotions[4].weight));
			disgustMorphMapWeights[i].push_back(vec2(j, emotions[5].weight));
			fearMorphMapWeights[i].push_back(vec2(j, emotions[6].weight));
		}
	}

    emotions[0].morphTargetsMapInfluences = neutralMorphMapWeights;
    emotions[1].morphTargetsMapInfluences = happinessMorphMapWeights;
    emotions[2].morphTargetsMapInfluences = sadnessMorphMapWeights;
    emotions[3].morphTargetsMapInfluences = surpriseMorphMapWeights;
    emotions[4].morphTargetsMapInfluences = angerMorphMapWeights;
    emotions[5].morphTargetsMapInfluences = disgustMorphMapWeights;
    emotions[6].morphTargetsMapInfluences = fearMorphMapWeights;

	// Init the Valence-Arousal model grid, precomputing the weights for each cell using Voronoid interpolation
    std::vector<vec2> points;
	for (unsigned int i = 0; i < emotions.size(); i++) 
	{
		points.push_back(emotions[i].position);
	}
	precomputedVAweights.resize(points.size());
    precomputeVoronoi(GRID_SIZE, points, precomputedVAweights);
	
}

void Lab6::createGazeSolver() {
	
	// [CA] To do: 

	// Find the index of the head joint from the entity skeleton and store it into the entity
	std::vector<std::string> entityJointNames = entity.skeleton.getJointNames();

	for (int i = 0; i < entityJointNames.size(); i++)
	{
		if (entityJointNames[i] == "mixamorig_Head")
		{
			entity.headJointIdx = i;
			break;
		}
	}
	
	// Init the target gaze position
	gazeTarget = entity.pose.getGlobalTransform(entity.headJointIdx);

	// Create visual target gaze gizmo
	targetVisual[0] = new DebugDraw(2);
	targetVisual[1] = new DebugDraw(2);
	targetVisual[2] = new DebugDraw(2);
}

void Lab6::render(float inAspectRatio) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glClearColor(0.19f, 0.19f, 0.19f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glPointSize(5.0f);

	camera->setPerspective(camera->fov, inAspectRatio, 0.01f, 1000.0f);
	mat4 view_projection = camera->getViewProjectionMatrix();
	
	shader->Bind();

	// Send camera info
	Uniform<vec3>::Set(shader->GetUniform("camPos"), camera->eye);
	Uniform<mat4>::Set(shader->GetUniform("view_projection"), view_projection);

	// Send light info
	Uniform<vec3>::Set(shader->GetUniform("lightPos"), vec3(0.5, 2, 1));
	Uniform<vec3>::Set(shader->GetUniform("light"), vec3(3.5));
	Uniform<vec3>::Set(shader->GetUniform("ambientLight"), vec3(0.04));
	
	// Send data for skinning
	std::vector<mat4> poseMatrices;
	if (showBindPose) {
		poseMatrices = entity.skeleton.getBindPose().getGlobalMatrices();
	}
	else {
		poseMatrices = entity.pose.getGlobalMatrices(); 
	}

	Uniform<mat4>::Set(shader->GetUniform("model"), entity.model);
	Uniform<mat4>::Set(shader->GetUniform("pose"), poseMatrices);
	Uniform<mat4>::Set(shader->GetUniform("invBindPose"), entity.skeleton.getInvBindPose());

	// Render each mesh of the entity
	for (unsigned int i = 0, size = (unsigned int)entity.meshes.size(); i < size; ++i) {
		
		// Send num of mesh vertices
		Uniform<int>::Set(shader->GetUniform("numVertices"), entity.meshes[i].getPositions().size());
		
		// Send encoded morph target data
		DataTexture* morphTargetTexture = entity.meshes[i].getMorphTargetsAtlas();
		morphTargetTexture->Set(shader->GetUniform("morphTargetsTexture"), 0);

		int sizeText = morphTargetTexture->Size();
		Uniform<ivec2>::Set(shader->GetUniform("morphTargetsTextureSize"), ivec2(sizeText));

		// Send morph target weights
		if (entity.morphTargetInfluences.size() && entity.morphTargetInfluences[i].size()) {

			Uniform<float>::Set(shader->GetUniform("morphTargetInfluences"), entity.morphTargetInfluences[i]);
			Uniform<int>::Set(shader->GetUniform("numMorphTargets"), entity.morphTargetInfluences[i].size());
		}

		// Send material properties
		Material material = entity.meshes[i].getMaterial();
	
		if (material.alpha_mode == alphaMode::ALPHA_BLEND) {
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		}
		if (material.alpha_mode == alphaMode::ALPHA_MASK) {
			glEnable(GL_BLEND);
			glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA);
		}
		Texture* texture = material.colorTexture;
		if (texture != NULL) {
			texture->Set(shader->GetUniform("colorTex"), 1);
		}

		Texture* normalTexture = material.normalMap;
		if (normalTexture != NULL) {
			normalTexture->Set(shader->GetUniform("normalMap"), 2);
		}

		Texture* metallicTexture = material.metallicTexture;
		if (metallicTexture != NULL) {
			metallicTexture->Set(shader->GetUniform("metallicTex"), 3);
		}


		Uniform<float>::Set(shader->GetUniform("metallicFactor"), material.metallic);
		Uniform<float>::Set(shader->GetUniform("specularFactor"), material.specular);
		Uniform<float>::Set(shader->GetUniform("alpha_cutoff"), material.alpha_cutoff);
		
		entity.meshes[i].bind(shader->GetAttribute("position"), shader->GetAttribute("normal"), shader->GetAttribute("texCoord"), shader->GetAttribute("weights"), shader->GetAttribute("joints"));
		entity.meshes[i].draw();
		entity.meshes[i].unBind(shader->GetAttribute("position"), shader->GetAttribute("normal"), shader->GetAttribute("texCoord"), shader->GetAttribute("weights"), shader->GetAttribute("joints"));
		
		morphTargetTexture->UnSet(0);
		if (texture != NULL) {
			texture->UnSet(1);
		}
		if (normalTexture != NULL) {
			normalTexture->UnSet(2);
		}	
		if (metallicTexture != NULL) {
			metallicTexture->UnSet(3);
		}
		if (material.double_side) {
			glDisable(GL_CULL_FACE);
		}
		if (material.alpha_mode == alphaMode::ALPHA_BLEND || material.alpha_mode == alphaMode::ALPHA_MASK) {
			glDisable(GL_BLEND);
		}
	}
	shader->UnBind();

	if (showSkeleton) {
		glDisable(GL_DEPTH_TEST);

		entity.skeletonHelper->fromPose(entity.pose);
		entity.skeletonHelper->updateOpenGLBuffers();
		entity.skeletonHelper->draw(DebugDrawMode::Lines, vec3(0, 1, 1), view_projection * entity.model);
		entity.skeletonHelper->draw(DebugDrawMode::Points, vec3(0, 1, 1), view_projection * entity.model);

		glEnable(GL_DEPTH_TEST);
	}


	if (showAxes) {
		glDisable(GL_DEPTH_TEST);
		mUpAxis->draw(DebugDrawMode::Lines, vec3(0, 1, 0), view_projection);
		mRightAxis->draw(DebugDrawMode::Lines, vec3(1, 0, 0), view_projection);
		mForwardAxis->draw(DebugDrawMode::Lines, vec3(0, 0, 1), view_projection);
		glEnable(GL_DEPTH_TEST);
	}

	if (currentTask == TASK3) {
		// FABRIK chain target visualization
		(*targetVisual[0])[0] = gazeTarget.position + vec3(GIZMO_SIZE, 0, 0);
		(*targetVisual[1])[0] = gazeTarget.position + vec3(0, GIZMO_SIZE, 0);
		(*targetVisual[2])[0] = gazeTarget.position + vec3(0, 0, GIZMO_SIZE);
		(*targetVisual[0])[1] = gazeTarget.position - vec3(GIZMO_SIZE, 0, 0);
		(*targetVisual[1])[1] = gazeTarget.position - vec3(0, GIZMO_SIZE, 0);
		(*targetVisual[2])[1] = gazeTarget.position - vec3(0, 0, GIZMO_SIZE);
		targetVisual[0]->updateOpenGLBuffers();
		targetVisual[1]->updateOpenGLBuffers();
		targetVisual[2]->updateOpenGLBuffers();
		targetVisual[0]->draw(DebugDrawMode::Lines, vec3(1, 0, 0), view_projection);
		targetVisual[1]->draw(DebugDrawMode::Lines, vec3(0, 1, 0), view_projection);
		targetVisual[2]->draw(DebugDrawMode::Lines, vec3(0, 0, 1), view_projection);

	}
}

void Lab6::update(float inDeltaTime) {


	if (currentTask == TASK2)
	{

		if (useVA) { // If Valence-Arousal model is used for interpolation
	
			// Compute the weights of the interpolated emotions based on the current position using Voronoi interpolation	
			std::vector<float> weights(emotions.size());
			interpolateVoronoi(GRID_SIZE, currentVA, precomputedVAweights, weights);
			
			// [CA] To do: Assign the computed interpolated weights to each emotion
			for (int i = 0; i < emotions.size(); i++) 
			{
				emotions[i].weight = weights[i];
			}
			
		}

		// Update the morph target influences for each mesh of the entity using the interpolated emotion weights
		for (unsigned int i = 0; i < entity.morphTargetInfluences.size(); i++) {
			
			std::fill(entity.morphTargetInfluences[i].begin(), entity.morphTargetInfluences[i].end(), 0);
			
			// [CA] To do: Compute the morph targets influences taking into account the weight of each emotion
			// For each emotion, get the morph targets map influences, scale them using the emotion weight, and accumulate them to the entity morph targets influences
			
			for (int j = 0; j < emotions.size(); j++)
			{
				// morph targets map influences
				std::vector<std::vector<vec2>> emotionMorphMapWeights = emotions[j].morphTargetsMapInfluences;

				for (int k = 0; k < emotionMorphMapWeights[i].size(); k++) 
				{
					// scale
					int morphTargetIndex = emotionMorphMapWeights[i][k].x;
					float influence = emotionMorphMapWeights[i][k].y * emotions[j].weight;

					// accumulate
					entity.morphTargetInfluences[i][morphTargetIndex] += influence;
				}
			}
			
		}
	}
	
	// [CA] To do: Gaze update
	if (currentTask == TASK3) 
	{
		
		// Get the global transformation of the head joint
		Transform globalHead = entity.pose.getGlobalTransform(entity.headJointIdx);

		// Compute the direction from the head joint to the gaze target
		vec3 headGazeDirection = gazeTarget.position - globalHead.position;
		normalize(headGazeDirection);
		
		// Find the global rotation given a direction vector and an orientation (up vector). 
		quat globalRotation = lookRotation(headGazeDirection, vec3(0, 1, 0));

		// Compute the new local rotation for the head joint and update its transform for the entity pose
		Transform localHead;
		localHead.rotation = globalRotation;
		localHead.position = localHead.position + vec3(0, 9, 0); //fix

		if (entity.headJointIdx > -1) 
		{
			entity.pose.setLocalTransform(entity.headJointIdx, localHead);
		}
		
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
void Lab6::ImGui(nk_context* context) {

    set_style(context, theme::THEME_RED);

	if (nk_begin(context, "Lab 6 Controls", nk_rect(5.0f, 70.0f, 300.0f, 650.0f), NK_WINDOW_MINIMIZABLE | NK_WINDOW_MOVABLE)) {

        if (nk_tree_push(context, NK_TREE_TAB, "Debug", NK_MINIMIZED)) {
            nk_layout_row_dynamic(context, 25, 1);
            nk_checkbox_label(context, "Show axes", &showAxes);
            nk_layout_row_dynamic(context, 25, 1);
            nk_checkbox_label(context, "Show skeletons", &showSkeleton);
            nk_layout_row_dynamic(context, 25, 1);
            nk_checkbox_label(context, "Apply bind pose", &showBindPose);
            nk_tree_pop(context);
        }

        if (nk_tree_push(context, NK_TREE_TAB, "Transformations", NK_MINIMIZED)) {
		    Transform transform = mat4ToTransform(entity.model);
		    vec3& position = transform.position;
		    nk_label(context, "Position", NK_TEXT_CENTERED);
		    nk_layout_row_dynamic(context, 25, 3);
		    nk_property_float(context, "#x", -5, &position.x, 5, 0.1, 0.01);
		    nk_property_float(context, "#y", -5, &position.y, 5, 0.1, 0.01);
		    nk_property_float(context, "#z", -5, &position.z, 5, 0.1, 0.01);

		    quat& rotation = transform.rotation;
		    nk_label(context, "Rotation", NK_TEXT_CENTERED);
		    nk_layout_row_dynamic(context, 25, 4);
		    nk_property_float(context, "#x", 0, &rotation.x, 1, 0.1, 0.01);
		    nk_property_float(context, "#y", 0, &rotation.y, 1, 0.1, 0.01);
		    nk_property_float(context, "#z", 0, &rotation.z, 1, 0.1, 0.01);
		    nk_property_float(context, "#w", 0, &rotation.w, 1, 0.1, 0.01);
		    normalize(rotation);

		    vec3& scale = transform.scale;
		    nk_label(context, "Scale", NK_TEXT_CENTERED);
		    nk_layout_row_dynamic(context, 25, 1);
		    nk_property_float(context, "#scale", 0.01, &scale.x, 10, 0.1, 0.01);
		    scale.y = scale.z = scale.x;

		    entity.model = transformToMat4(transform);
            nk_tree_pop(context);
        }

		// ------------------------------- Task selector GUI ------------------------------- //
		nk_layout_row_dynamic(context, 25, 1);
		nk_label(context, "Tasks", NK_TEXT_CENTERED);
		int task = nk_combo(context, tasks, NK_LEN(tasks), currentTask, 25, nk_vec2(200, 200));

		if (task != currentTask) {
			currentTask = task;

			// Set the bind pose as current pose
			
			entity.pose = entity.skeleton.getRestPose();
			
		}

		if (currentTask == TASK1) {
			// ------------------------------- Morph Targets GUI ------------------------------- //
			if (entity.morphTargetNames.size()) 
			{
				nk_layout_row_dynamic(context, 25, 1);
				nk_label(context, "Morph Targets", NK_TEXT_CENTERED);

				for (unsigned int m = 0; m < entity.morphTargetNames.size(); m++) 
				{
					if (!entity.morphTargetNames[m].size()) 
					{
						continue;
					}
					if (nk_tree_push(context, NK_TREE_NODE, strdup(entity.meshes[m].name.c_str()), NK_MINIMIZED)) 
					{
						for (unsigned int i = 0; i < entity.morphTargetNames[m].size(); i++) 
						{
							const char* name = strdup(entity.morphTargetNames[m][i].c_str());
							nk_property_float(context, name, 0.0, &entity.morphTargetInfluences[m][i], 1, 0.001, 0.1);
						}
						nk_tree_pop(context);
					}
				}
				nk_layout_row_dynamic(context, 25, 1);

			}
		}

		// ------------------------------- Emotions GUI ------------------------------- //
		if (currentTask == TASK2) 
		{
			nk_layout_row_dynamic(context, 25, 1);
			nk_checkbox_label(context, "Valence-Arousal", &useVA);

			if (!useVA) 
			{

				nk_label(context, "Emotions", NK_TEXT_CENTERED);
				for (unsigned int i = 1; i < emotions.size(); i++) 
				{

					nk_property_float(context, strdup(emotions[i].name.c_str()), 0.0, &emotions[i].weight, 1, 0.001, 0.1);
				}
		
			}
		}

		// ------------------------------- Gaze GUI ------------------------------- //
		if (currentTask == TASK3) 
		{

			nk_layout_row_dynamic(context, 25, 1);
			nk_label(context, "Gaze", NK_TEXT_CENTERED);
			nk_property_float(context, "#x", -10, &gazeTarget.position.x, 10, 0.001, 0.1);
			nk_property_float(context, "#y", -10, &gazeTarget.position.y, 10, 0.001, 0.1);
			nk_property_float(context, "#z", -10, &gazeTarget.position.z, 10, 0.001, 0.1);
			
		}
        
        activeScroll = !nk_window_is_hovered(context);
	    nk_end(context);
	}

	if (useVA && (currentTask == TASK2)) 
	{
	
		// Valence-Arousal GUI

		struct nk_canvas canvas;
		int size = 250;

		if (canvas_begin(context, &canvas, NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
			NK_WINDOW_CLOSABLE | NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE, 10, 250, size, size, context->style.window.background))
		{
			size = canvas.painter->clip.w;
			float x = canvas.painter->clip.x, y = canvas.painter->clip.y;
			nk_color textColor = context->style.text.color;
			nk_color widgetColor = context->style.window.border_color;
			nk_fill_rect(canvas.painter, nk_rect(x + size * 0.5 - 1, y, 2, size), 5, widgetColor);
			nk_fill_rect(canvas.painter, nk_rect(x, y + size * 0.5 - 1, size, 2), 5, widgetColor);


			if (nk_input_mouse_clicked(&context->input, NK_BUTTON_LEFT, canvas.painter->clip)) {
				currentVA.x = (context->input.mouse.pos.x - x) * 2 / size - 1;
				currentVA.y = -(context->input.mouse.pos.y - y) * 2 / size + 1;

			}
			// Current
			nk_draw_text(canvas.painter, nk_rect(x + (currentVA.x + 1) * size * 0.5 + 5, y + (1 - currentVA.y) * size * 0.5 + 5, 150, 20), strdup("Current"), 12, context->style.font, nk_rgb(188, 174, 118), textColor);
			nk_fill_circle(canvas.painter, nk_rect(x + (currentVA.x + 1) * size * 0.5, y + (1 - currentVA.y) * size * 0.5, 5, 5), nk_rgb(255, 0, 255));

			for (unsigned int i = 0; i < emotions.size(); i++) {
				Emotion emotion = emotions[i];
				nk_draw_text(canvas.painter, nk_rect(x + (emotion.position.x + 1) * size * 0.5 + 5, y + (1 - emotion.position.y) * size * 0.5 + 5, 150, 20), strdup(emotion.name.c_str()), 12, context->style.font, nk_rgb(188, 174, 118), textColor);
				nk_fill_circle(canvas.painter, nk_rect(x + (emotion.position.x + 1) * size * 0.5, y + (1 - emotion.position.y) * size * 0.5, 5, 5), nk_rgb(255, 255, 255));
			}

		}
		canvas_end(context, &canvas);
	}
}


void Lab6::precomputeVoronoi(const int gridSize, const std::vector<vec2>& points, std::vector<float>& values) {

	const int numPoints = points.size();
	values.resize(2 * gridSize * gridSize);

	for (int i = 0; i < gridSize; ++i) {
		for (int j = 0; j < gridSize; ++j) {


			vec2 midpoint;

			//normalize
			midpoint.x = i / (float)gridSize;
			midpoint.y = j / (float)gridSize;
			// center coords
			midpoint.x = midpoint.x * 2 - 1;
			midpoint.y = midpoint.y * 2 - 1;
			double minDist = INFINITY;
			int nearestSite = -1;

			for (int k = 0; k < numPoints; ++k) {

				const vec2& p = points[k];
				float dist = distance(midpoint, p);

				if (dist < minDist) {
					minDist = dist;
					nearestSite = k;
				}
			}

			values[2 * (i + j * gridSize)] = nearestSite;
			values[2 * (i + j * gridSize) + 1] = minDist;
		}
	}
}

void Lab6::interpolateVoronoi(int gridSize, vec2 point, std::vector<float> values, std::vector<float>& weights) {

	int totalInside = 0;

	vec2 position = point;

	std::fill(weights.begin(), weights.end(), 0.0);


	// For each position in grid, check if distance to pos is lower than distance to its nearest neighbourg
	for (unsigned int i = 0; i < gridSize; ++i) {
		for (unsigned int j = 0; j < gridSize; ++j) {

			// Normalize
			position.x = i / (float)gridSize;
			position.y = j / (float)gridSize;
			// Convert the position into the range [-1,1]
			position.x = position.x * 2 - 1;
			position.y = position.y * 2 - 1;

			float data_pos = (i + j * gridSize) * 2; // two values in each entry
			float point_index = values[data_pos];
			float point_distance = values[data_pos + 1];
			float is_inside = distance(position, point) < (point_distance + 0.001); //epsilon
			if (is_inside) {
				weights[point_index] += 1;
				totalInside++;
			}
		}
	}
	if (!totalInside)
		return;

	// Average each weight with respect to amount of points near the current point
	for (unsigned int i = 0; i < weights.size(); ++i) {
		weights[i] = weights[i] / totalInside;
	}
}

void Lab6::shutdown() {
	delete mUpAxis;
	delete mRightAxis;
	delete mForwardAxis;

	delete entity.skeletonHelper;

	delete targetVisual[0];
	delete targetVisual[1];
	delete targetVisual[2];

}

void Lab6::onKeyDown(int key, int scancode) {
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

void Lab6::onKeyUp(int key, int scancode) {
	// keycodes: https://www.glfw.org/docs/3.3/group__keys.html
	switch (key) {
		case GLFW_KEY_T:
			std::cout << "T released" << std::endl;
			break;
	}
};

void Lab6::onRightMouseButtonDown() {
	dragging = true;
	lastMousePosition = mousePosition;
}

void Lab6::onRightMouseButtonUp() {
	dragging = false;
	lastMousePosition = mousePosition;
};

void Lab6::onLeftMouseButtonDown() { };

void Lab6::onLeftMouseButtonUp() { };

void Lab6::onMiddleMouseButtonDown() {
	moving = true;
};

void Lab6::onMiddleMouseButtonUp() {
	moving = false;
};

void Lab6::onScroll(double xOffset, double yOffset) {

    if (activeScroll) {

	    float scale = yOffset > 0 ? 0.9 : 1.1;
	    camera->zoom(scale);
    }
};