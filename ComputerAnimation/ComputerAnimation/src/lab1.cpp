#include <GLFW/glfw3.h>
#include <windows.h>
#include <iostream>

#include "lab1.h"
#include "loaders/gLTFLoader.h"
#include "shading/uniform.h"

const char* Lab1::tasks[] = {"Dot", "Cross", "Inverse Matrix", "Quats and Mats", "Transforms", "Interpolation"};

void Lab1::init() {

	// TODO: Init here all your attributes

	v1 = { 1.0, 2.0, 3.0 };
	v2 = { 4.0, 5.0, 6.0 };

	matrix = { 2, 1, 3, 0,
				   1, 0, 2, 1,
				   0, 2, 1, 2,
				   3, -1, 4, 3 };

	qauternion = quat{ 0.0, 0.0, 0.707, 0.707 };

	scale = 2.0f;

	duration = 1;

	//axes
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

	//push vertex cube indices to the shader
	cubeDraw = new DebugDraw();
	cubeDraw->push(vec3(-1.0f, -1.0f, -1.0f)); // triangle 1 : begin
	cubeDraw->push(vec3(-1.0f, -1.0f, 1.0f));
	cubeDraw->push(vec3(-1.0f, 1.0f, 1.0f)); // triangle 1 : end
	cubeDraw->push(vec3(1.0f, 1.0f, -1.0f)); // triangle 2 : begin
	cubeDraw->push(vec3(-1.0f, -1.0f, -1.0f));
	cubeDraw->push(vec3(-1.0f, 1.0f, -1.0f)); // triangle 2 : end
	cubeDraw->push(vec3(1.0f, -1.0f, 1.0f));
	cubeDraw->push(vec3(-1.0f, -1.0f, -1.0f));
	cubeDraw->push(vec3(1.0f, -1.0f, -1.0f));
	cubeDraw->push(vec3(1.0f, 1.0f, -1.0f));
	cubeDraw->push(vec3(1.0f, -1.0f, -1.0f));
	cubeDraw->push(vec3(-1.0f, -1.0f, -1.0f));
	cubeDraw->push(vec3(-1.0f, -1.0f, -1.0f));
	cubeDraw->push(vec3(-1.0f, 1.0f, 1.0f));
	cubeDraw->push(vec3(-1.0f, 1.0f, -1.0f));
	cubeDraw->push(vec3(1.0f, -1.0f, 1.0f));
	cubeDraw->push(vec3(-1.0f, -1.0f, 1.0f));
	cubeDraw->push(vec3(-1.0f, -1.0f, -1.0f));
	cubeDraw->push(vec3(-1.0f, 1.0f, 1.0f));
	cubeDraw->push(vec3(-1.0f, -1.0f, 1.0f));
	cubeDraw->push(vec3(1.0f, -1.0f, 1.0f));
	cubeDraw->push(vec3(1.0f, 1.0f, 1.0f));
	cubeDraw->push(vec3(1.0f, -1.0f, -1.0f));
	cubeDraw->push(vec3(1.0f, 1.0f, -1.0f));
	cubeDraw->push(vec3(1.0f, -1.0f, -1.0f));
	cubeDraw->push(vec3(1.0f, 1.0f, 1.0f));
	cubeDraw->push(vec3(1.0f, -1.0f, 1.0f));
	cubeDraw->push(vec3(1.0f, 1.0f, 1.0f));
	cubeDraw->push(vec3(1.0f, 1.0f, -1.0f));
	cubeDraw->push(vec3(-1.0f, 1.0f, -1.0f));
	cubeDraw->push(vec3(1.0f, 1.0f, 1.0f));
	cubeDraw->push(vec3(-1.0f, 1.0f, -1.0f));
	cubeDraw->push(vec3(-1.0f, 1.0f, 1.0f));
	cubeDraw->push(vec3(1.0f, 1.0f, 1.0f));
	cubeDraw->push(vec3(-1.0f, 1.0f, 1.0f));
	cubeDraw->push(vec3(1.0f, -1.0f, 1.0f));

	cubeDraw->updateOpenGLBuffers();
}

void Lab1::render(float inAspectRatio) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glClearColor(0.5f, 0.6f, 0.7f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glPointSize(5.0f);

	mat4 projection = perspective(60.0f, inAspectRatio, 0.01f, 1000.0f);;
	mat4 view = lookAt(vec3(0, 0, 10), vec3(0,0,0), vec3(0, 1, 0));
	mat4 model = transformToMat4(t);

	switch (currentTask) {

	case DOT:
		break;
	case TRANSFORMS: case INTERPOLATION:
		{	
			cubeDraw->draw(DebugDrawMode::Triangles, vec3(1, 0, 0), projection * view * model);
		}
		break;
	}

	if (showAxes) {
		mUpAxis->draw(DebugDrawMode::Lines, vec3(0, 1, 0), projection * view);
		mRightAxis->draw(DebugDrawMode::Lines, vec3(1, 0, 0), projection * view);
		mForwardAxis->draw(DebugDrawMode::Lines, vec3(0, 0, 1), projection * view);
	}
}

void Lab1::update(float inDeltaTime) {

	// TODO: perform here all your operations

	switch (currentTask) {

		case TRANSFORMS:
		{
			t.position = vec3{ x_translation, y_translation, z_translation };
			alpha_mat = { cos(alpha), -sin(alpha), 0, 0,
						  sin(alpha), cos(alpha), 0, 0,
						  0, 0, 1, 0,
						  0, 0, 0, 0 }; //z roll
			beta_mat = { cos(beta), 0, sin(beta), 0,
						 0, 1, 0, 0,
						 -sin(beta), 0, cos(beta), 0,
						 0, 0, 0, 0 }; //y yaw
			gamma_mat = { 1, 0, 0, 0,
						  0, cos(gamma), -sin(gamma), 0,
						  0, sin(gamma), cos(gamma), 0,
						  0, 0, 0, 0 }; //x pitch	

			mat4 rotatiton_mat = alpha_mat * beta_mat * gamma_mat;
			t.rotation = mat4ToQuat(rotatiton_mat);
			t.scale = vec3{ scale };
	
			break;
		}
		case INTERPOLATION:
		{
			if (play == false || restart == true)
			{
				t.position = initPos;

				alpha_mat = { cos(initAngle.x), -sin(initAngle.x), 0, 0,
								   sin(initAngle.x), cos(initAngle.x), 0, 0,
								   0, 0, 1, 0,
								   0, 0, 0, 0 }; //z roll
				beta_mat = { cos(initAngle.y), 0, sin(initAngle.y), 0,
							   0, 1, 0, 0,
							  -sin(initAngle.y), 0, cos(initAngle.y), 0,
							   0, 0, 0, 0 }; //y yaw
				gamma_mat = { 1, 0, 0, 0,
							  0, cos(initAngle.z), -sin(initAngle.z), 0,
							  0, sin(initAngle.z), cos(initAngle.z), 0,
							  0, 0, 0, 0 }; //x pitch			
			}

			if (play == true)
			{
				if (lerpTime < 1.0f) // If lerpTime is less than 1, perform interpolation
				{
					vec3 pos = lerp(initPos, finalPos, lerpTime);
					t.position = pos;

					vec3 angle = lerp(initAngle, finalAngle, lerpTime);

					alpha_mat = { cos(angle.x), -sin(angle.x), 0, 0,
								   sin(angle.x), cos(angle.x), 0, 0,
								   0, 0, 1, 0,
								   0, 0, 0, 0 }; //z roll
					beta_mat = { cos(angle.y), 0, sin(angle.y), 0,
								   0, 1, 0, 0,
								  -sin(angle.y), 0, cos(angle.y), 0,
								   0, 0, 0, 0 }; //y yaw
					gamma_mat = { 1, 0, 0, 0,
								  0, cos(angle.z), -sin(angle.z), 0,
								  0, sin(angle.z), cos(angle.z), 0,
								  0, 0, 0, 0 }; //x pitch

					lerpTime += inDeltaTime / duration;
				}
				else 
				{
					t.position = finalPos;

					alpha_mat = { cos(finalAngle.x), -sin(finalAngle.x), 0, 0,
								   sin(finalAngle.x), cos(finalAngle.x), 0, 0,
								   0, 0, 1, 0,
								   0, 0, 0, 0 }; //z roll
					beta_mat = { cos(finalAngle.y), 0, sin(finalAngle.y), 0,
								   0, 1, 0, 0,
								  -sin(finalAngle.y), 0, cos(finalAngle.y), 0,
								   0, 0, 0, 0 }; //y yaw
					gamma_mat = { 1, 0, 0, 0,
								  0, cos(finalAngle.z), -sin(finalAngle.z), 0,
								  0, sin(finalAngle.z), cos(finalAngle.z), 0,
								  0, 0, 0, 0 }; //x pitch		
				}
			}

			mat4 rotatiton_mat = alpha_mat * beta_mat * gamma_mat;
			t.rotation = mat4ToQuat(rotatiton_mat);
			t.scale = vec3{ scale };
			break;
		}
	}

	
	// quit
	if (GetAsyncKeyState(VK_ESCAPE)) close = true;
}

//https://github.com/vurtun/nuklear/blob/master/example/extended.c
void Lab1::ImGui(nk_context* context) {

	static float range_float_min = -10;
	static float range_float_max = 10;

	char d[20];

	if (nk_begin(context, "Lab 1 Controls", nk_rect(5.0f, 70.0f, 300.0f, 300.0f), NK_WINDOW_MINIMIZABLE | NK_WINDOW_MOVABLE)) 
	{
		nk_layout_row_static(context, 25, 200, 1);
		currentTask = nk_combo(context, tasks, NK_LEN(tasks), currentTask, 25, nk_vec2(200, 200));
		nk_layout_row_static(context, 25, 200, 1);
		nk_checkbox_label(context, "Show axes", &showAxes);

		switch (currentTask) {
			case DOT:
			{
				nk_layout_row_dynamic(context, 30, 1);
				nk_label_colored(context, "v2", NK_TEXT_LEFT, nk_rgb(0, 125, 0));

				nk_layout_row_dynamic(context, 30, 1);
				nk_property_float(context, "#x", range_float_min, &v2.x, range_float_max, 0.01, 0.2f);

				nk_layout_row_dynamic(context, 30, 1);
				nk_property_float(context, "#y", range_float_min, &v2.y, range_float_max, 0.01, 0.2f);

				nk_layout_row_dynamic(context, 30, 2);

				sprintf_s(d, "%0.3f", dot(v1, v2));
				nk_label(context, "Dot product", NK_TEXT_LEFT);
				nk_text(context, d, 4, NK_TEXT_RIGHT);

				break;
			}

			case CROSS:
			{
				nk_layout_row_dynamic(context, 30, 1);
				nk_label_colored(context, "v1", NK_TEXT_LEFT, nk_rgb(255, 255, 0));
				nk_layout_row_dynamic(context, 30, 3);
				nk_property_float(context, "#x", range_float_min, &v1.x, range_float_max, 0.01, 0.2f);
				nk_property_float(context, "#y", range_float_min, &v1.y, range_float_max, 0.01, 0.2f);
				nk_property_float(context, "#z", range_float_min, &v1.z, range_float_max, 0.01, 0.2f);

				nk_layout_row_dynamic(context, 30, 1);
				nk_label_colored(context, "v2", NK_TEXT_LEFT, nk_rgb(0, 125, 0));
				nk_layout_row_dynamic(context, 30, 3);
				nk_property_float(context, "#x", range_float_min, &v2.x, range_float_max, 0.01, 0.2f);
				nk_property_float(context, "#y", range_float_min, &v2.y, range_float_max, 0.01, 0.2f);
				nk_property_float(context, "#z", range_float_min, &v2.z, range_float_max, 0.01, 0.2f);

				//sprintf_s(d, "%0.3f, %0.3f, %0.3f", cross(v1, v2).x, cross(v1, v2).y, cross(v1, v2).x);
				nk_label(context, "Cross product", NK_TEXT_LEFT);
				nk_layout_row_dynamic(context, 30, 3);
				for (int i = 0; i < 3; ++i) {
					//convert element value into a string
					sprintf_s(d, "%f", cross(v1, v2).v[i]);
					nk_selectable_label(context, d, NK_TEXT_CENTERED, (int*)(&cross(v1, v2).v[i]));
				}
				break;
			}

			case INV_MAT:
			{
				nk_layout_row_dynamic(context, 30, 3);
				nk_label(context, "Input Matrix", NK_TEXT_RIGHT);
				nk_layout_row_static(context, 50, 60, 4);
				//go through 16 elements of the matrix
				for (int i = 0; i < 16; ++i) {
					nk_property_float(context, "", range_float_min, &matrix.v[i], range_float_max, 0.25, 0.2f);
				}
				nk_layout_row_dynamic(context, 30, 3);
				nk_label(context, "Inverse Matrix", NK_TEXT_RIGHT);
				nk_layout_row_static(context, 50, 50, 4);
				//go through 16 elements of the matrix
				for (int i = 0; i < 16; ++i) {
					sprintf_s(d, "%f", inverse(matrix).v[i]);
					nk_selectable_label(context, d, NK_TEXT_CENTERED, (int*)(&inverse(matrix).v[i]));
				}
				break;
			}

			case QUAT_MAT: 
			{
				nk_layout_row_dynamic(context, 30, 3);
				nk_label(context, "Input Quaternion", NK_TEXT_RIGHT);
				nk_layout_row_static(context, 50, 65, 4);
				//go through 4 elements of the quaternion
				for (int i = 0; i < 4; ++i) {
					nk_property_float(context, "", range_float_min, &qauternion.v[i], range_float_max, 0.1, 0.2f);
				}
				nk_layout_row_dynamic(context, 30, 3);
				nk_label(context, "Respective Matrix", NK_TEXT_RIGHT);
				nk_layout_row_static(context, 50, 50, 4);
				//go through 16 elements of the matrix
				for (int i = 0; i < 16; ++i) {
					//convert element value into a string
					sprintf_s(d, "%f", quatToMat4(qauternion).v[i]);
					nk_selectable_label(context, d, NK_TEXT_CENTERED, (int*)(&quatToMat4(qauternion).v[i]));
				}
				break;
			}

			case TRANSFORMS: 
			{
				nk_layout_row_dynamic(context, 30, 3);
				nk_label(context, "Scale", NK_TEXT_LEFT);
				nk_slider_float(context, 0.0f, &scale, 10.0f, 0.1f);
				sprintf_s(d, "%0.3f", scale);
				nk_text(context, d, 4, NK_TEXT_LEFT);

				nk_layout_row_dynamic(context, 30, 3);
				nk_label(context, "Pitch", NK_TEXT_LEFT);
				nk_slider_float(context, -5.0f, &alpha, 5.0f, 0.01f);
				sprintf_s(d, "%0.3f", alpha);
				nk_text(context, d, 4, NK_TEXT_LEFT);

				nk_layout_row_dynamic(context, 30, 3);
				nk_label(context, "Yaw", NK_TEXT_LEFT);
				nk_slider_float(context, -5.0f, &beta, 5.0f, 0.01f);
				sprintf_s(d, "%0.3f", beta);
				nk_text(context, d, 4, NK_TEXT_LEFT);

				nk_layout_row_dynamic(context, 30, 3);
				nk_label(context, "Roll", NK_TEXT_LEFT);
				nk_slider_float(context, -5.0f, &gamma, 5.0f, 0.01f);
				sprintf_s(d, "%0.3f", gamma);
				nk_text(context, d, 4, NK_TEXT_LEFT);

				nk_layout_row_dynamic(context, 30, 3);
				nk_label(context, "Translate X", NK_TEXT_LEFT);
				nk_slider_float(context, -30.0f, &x_translation, 30.0f, 0.01f);
				sprintf_s(d, "%0.3f", x_translation);
				nk_text(context, d, 4, NK_TEXT_LEFT);

				nk_layout_row_dynamic(context, 30, 3);
				nk_label(context, "Translate Y", NK_TEXT_LEFT);
				nk_slider_float(context, -30.0f, &y_translation, 30.0f, 0.01f);
				sprintf_s(d, "%0.3f", y_translation);
				nk_text(context, d, 4, NK_TEXT_LEFT);

				nk_layout_row_dynamic(context, 30, 3);
				nk_label(context, "Translate Z", NK_TEXT_LEFT);
				nk_slider_float(context, -30.0f, &z_translation, 30.0f, 0.01f);
				sprintf_s(d, "%0.3f", z_translation);
				nk_text(context, d, 4, NK_TEXT_LEFT);

				if (nk_button_label(context, "Reset"))
				{
					qauternion = quat{ 0.0, 0.0, 0.707, 0.707 };
					x_translation = 0.0f;
					y_translation = 0.0f;
					z_translation = 0.0f;
					scale = 2.0f;
					alpha = 0.0f;
					beta = 0.0f;
					gamma = 0.0f;
				}
				break;
			}

			case INTERPOLATION: 
			{
				nk_layout_row_dynamic(context, 30, 1);
				nk_label(context, "Duration", NK_TEXT_LEFT);
				nk_layout_row_dynamic(context, 30, 1);
				nk_property_float(context, "Duration", 0, &duration, nk_widget_width(context), 0.1, 0.2f);

				nk_layout_row_dynamic(context, 30, 3);
				nk_label(context, "Scale", NK_TEXT_LEFT);
				nk_slider_float(context, 0.0f, &scale, 10.0f, 0.1f);
				sprintf_s(d, "%0.3f", scale);
				nk_text(context, d, 4, NK_TEXT_LEFT);

				//change init position of the cube
				nk_layout_row_dynamic(context, 30, 1);
				nk_label(context, "Start position", NK_TEXT_LEFT);
				nk_layout_row_dynamic(context, 30, 3);
				nk_property_float(context, "#x", range_float_min, &initPos.x, range_float_max, 0.1, 0.2f);
				nk_property_float(context, "#y", range_float_min, &initPos.y, range_float_max, 0.1, 0.2f);
				nk_property_float(context, "#z", range_float_min, &initPos.z, range_float_max, 0.1, 0.2f);

				//change init rotation of the cube
				nk_layout_row_dynamic(context, 30, 1);
				nk_label(context, "Start rotation", NK_TEXT_LEFT);
				nk_layout_row_dynamic(context, 30, 3);
				nk_property_float(context, "#x", range_float_min, &initAngle.z, range_float_max, 0.1, 0.2f);
				nk_property_float(context, "#y", range_float_min, &initAngle.y, range_float_max, 0.1, 0.2f);
				nk_property_float(context, "#z", range_float_min, &initAngle.x, range_float_max, 0.1, 0.2f);

				//change final position of the cube
				nk_layout_row_dynamic(context, 30, 1);
				nk_label(context, "End position", NK_TEXT_LEFT);
				nk_layout_row_dynamic(context, 30, 3);
				nk_property_float(context, "#x", range_float_min, &finalPos.x, range_float_max, 0.1, 0.2f);
				nk_property_float(context, "#y", range_float_min, &finalPos.y, range_float_max, 0.1, 0.2f);
				nk_property_float(context, "#z", range_float_min, &finalPos.z, range_float_max, 0.1, 0.2f);

				//change final rotation of the cube
				nk_layout_row_dynamic(context, 30, 1);
				nk_label(context, "End rotatiob", NK_TEXT_LEFT);
				nk_layout_row_dynamic(context, 30, 3);
				nk_property_float(context, "#x", range_float_min, &finalAngle.z, range_float_max, 0.1, 0.2f);
				nk_property_float(context, "#y", range_float_min, &finalAngle.y, range_float_max, 0.1, 0.2f);
				nk_property_float(context, "#z",range_float_min, &finalAngle.x, range_float_max, 0.1, 0.2f);

				nk_layout_row_dynamic(context, 30, 1);

				if (nk_button_label(context, "Play")) 
				{
					play = true;
					lerpTime = 0;
				}
				if (nk_button_label(context, "Restart"))
				{
					play = false;
				}
				//reset the animation
				if (nk_button_label(context, "Reset")) 
				{			
					qauternion = quat{ 0.0, 0.0, 0.707, 0.707 };
					initPos = vec3(0.0f);
					finalPos = vec3(0.0f);
					initAngle = vec3(0.0f);
					finalAngle = vec3(0.0f);
					duration = 1;
					scale = 2.0f;
					play = false;
				}
				break;
			}
		}
	}

	nk_end(context);
}

void Lab1::shutdown() { }