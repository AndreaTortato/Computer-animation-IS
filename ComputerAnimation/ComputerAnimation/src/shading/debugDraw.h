#ifndef _H_DEBUGLINE_
#define _H_DEBUGLINE_

#include "shader.h"
#include "attribute.h"
#include "../math/vec3.h"
#include "../math/mat4.h"
#include "../animation/pose.h"
#include <vector>


enum class DebugDrawMode {
	Lines, Loop, Strip, Points, Triangles
};

class DebugDraw {
protected:
	std::vector<vec3> mPoints;
	Attribute<vec3>* mAttribs;
	Shader* mShader;
private:
	DebugDraw(const DebugDraw&);
	DebugDraw& operator=(const DebugDraw&);
public:
	DebugDraw();
	DebugDraw(unsigned int size);
	~DebugDraw();

	unsigned int size();
	void resize(unsigned int newSize);
	vec3& operator[](unsigned int index);
	void push(const vec3& v);

	void updateOpenGLBuffers();
	void draw(DebugDrawMode mode, const vec3& color, const mat4& mvp);
	void fromPose(Pose& pose);
};

#endif