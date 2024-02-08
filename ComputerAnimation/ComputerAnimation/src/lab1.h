#pragma once

#include "application.h"
#include "shading/debugDraw.h"
#include "../animation/pose.h"
#include "../animation/skeleton.h"
#include "camera.h"
#include "shading/texture.h"
#include "shading/shader.h"
#include "../shading/mesh.h"
#include "cgltf.h""


class Lab1 : public Application {
protected:
	enum tasks { DOT, CROSS, INV_MAT, QUAT_MAT, TRANSFORMS, INTERPOLATION };
	static const char* tasks[];

	int currentTask;

	int showAxes;

	DebugDraw* mUpAxis;
	DebugDraw* mRightAxis;
	DebugDraw* mForwardAxis;

	DebugDraw* cubeDraw;



	// TODO: create vectors for dot and cross product

	vec3 v1;
	vec3 v2;
	mat4 matrix;
	quat qauternion;

	float scale;
	float x_translation;
	float y_translation;
	float z_translation;

	Transform t;

	vec3 initPos;
	vec3 finalPos;
	vec3 initAngle; //{roll, yaw, pitch}
	vec3 finalAngle;
	float duration;
	bool play = false;
	bool restart = false;
	float lerpTime = 0;

	float alpha = 0.0f;
	float beta = 0.0f;
	float gamma = 0.0f;
	mat4 alpha_mat;
	mat4 beta_mat;
	mat4 gamma_mat;

public:
	void init();
	void update(float inDeltaTime);
	void render(float inAspectRatio);
	void ImGui(nk_context* context);
	void shutdown();
};
