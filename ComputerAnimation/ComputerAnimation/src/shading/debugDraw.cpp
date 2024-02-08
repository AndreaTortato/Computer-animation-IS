#include "debugDraw.h"
#include "uniform.h"
#include "draw.h"

DebugDraw::DebugDraw() {
	mAttribs = new Attribute<vec3>();

	mShader = new Shader(
		"#version 330 core\n"
		"uniform mat4 mvp;\n"
		"in vec3 position;\n"
		"void main() {\n"
		"	gl_Position = mvp * vec4(position, 1.0);\n"
		"}"
		,
		"#version 330 core\n"
		"uniform vec3 color;\n"
		"out vec4 FragColor;\n"
		"void main() {\n"
		"	FragColor = vec4(color, 1);\n"
		"}"
	);
}

DebugDraw::DebugDraw(unsigned int size) {
	mAttribs = new Attribute<vec3>();

	mShader = new Shader(
		"#version 330 core\n"
		"uniform mat4 mvp;\n"
		"in vec3 position;\n"
		"void main() {\n"
		"	gl_Position = mvp * vec4(position, 1.0);\n"
		"}"
		,
		"#version 330 core\n"
		"uniform vec3 color;\n"
		"out vec4 FragColor;\n"
		"void main() {\n"
		"	FragColor = vec4(color, 1);\n"
		"}"
	);

	resize(size);
}

DebugDraw::~DebugDraw() {
	delete mAttribs;
	delete mShader;
}

unsigned int DebugDraw::size() {
	return (unsigned int)mPoints.size();
}

void DebugDraw::resize(unsigned int newSize) {
	mPoints.resize(newSize);
}

vec3& DebugDraw::operator[](unsigned int index) {
	return mPoints[index];
}

void DebugDraw::push(const vec3& v) {
	mPoints.push_back(v);
}


void DebugDraw::updateOpenGLBuffers() {
	mAttribs->Set(mPoints);
}

void DebugDraw::draw(DebugDrawMode mode, const vec3& color, const mat4& mvp) {
	mShader->Bind();
	Uniform<mat4>::Set(mShader->GetUniform("mvp"), mvp);
	Uniform<vec3>::Set(mShader->GetUniform("color"), color);
	mAttribs->BindTo(mShader->GetAttribute("position"));
	if (mode == DebugDrawMode::Lines) {
		::Draw(size(), DrawMode::Lines);
	}
	else if (mode == DebugDrawMode::Loop) {
		::Draw(size(), DrawMode::LineLoop);
	}
	else if (mode == DebugDrawMode::Strip) {
		::Draw(size(), DrawMode::LineStrip);
	}
	else if (mode == DebugDrawMode::Points) {
		::Draw(size(), DrawMode::Points);
	}
	else {
		::Draw(size(), DrawMode::Triangles);
	}
	mAttribs->UnBindFrom(mShader->GetAttribute("position"));
	mShader->UnBind();
}

void DebugDraw::fromPose(Pose& pose) {
	unsigned int requiredVerts = 0;
	unsigned int numJoints = pose.size();
	for (unsigned int i = 0; i < numJoints; ++i) {
		if (pose.getParent(i) < 0) {
			continue;
		}

		requiredVerts += 2;
	}

	mPoints.resize(requiredVerts);
	for (unsigned int i = 0; i < numJoints; ++i) {
		if (pose.getParent(i) < 0) {
			continue;
		}

		mPoints.push_back(pose.getGlobalTransform(i).position);
		mPoints.push_back(pose.getGlobalTransform(pose.getParent(i)).position);
	}
}