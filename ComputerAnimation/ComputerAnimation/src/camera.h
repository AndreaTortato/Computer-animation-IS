#pragma once
#include "math/vec3.h"
#include "math/mat4.h"

class Camera
{
	// OpenGL methods to fill matrices
public:

	// Types of cameras available
	enum { PERSPECTIVE, ORTHOGRAPHIC };
	char type;

	// Vectors to define the orientation of the camera
	vec3 eye;	// Where is the camera
	vec3 center; // Where is it pointing
	vec3 up;		// The up pointing up

	// Properties of the projection of the camera
	float fov;			// View angle in degrees (1/zoom)
	float aspect;		// Aspect ratio (width/height)
	float near_plane;	// Near plane
	float far_plane;	// Far plane

	// For orthogonal projection
	float left, right, top, bottom;

	// Matrices
	mat4 view_matrix;
	mat4 projection_matrix;
	mat4 viewprojection_matrix;

	Camera();

	// Setters
	void setAspectRatio(float aspect) { this->aspect = aspect; };

	// Translate and rotate the camera
	void move(vec3 delta);
	void rotate(float angle, const vec3& axis);
	void orbit(float angle, const vec3& axis);
	void zoom(float scale);

	// Transform a local camera vector to world coordinates
	vec3 getLocalVector(const vec3& v);

	// Project 3D Vectors to 2D Homogeneous Space
	// If negZ is true, the projected point IS NOT inside the frustum, 
	// so it does not have to be rendered!
	vec3 projectVector(vec3 pos, bool& negZ);

	// Set the info for each projection
	void setPerspective(float fov, float aspect, float near_plane, float far_plane);
	void setOrthographic(float left, float right, float top, float bottom, float near_plane, float far_plane);
	void lookAt(const vec3& eye, const vec3& center, const vec3& up);

	// Compute the matrices
	void updateViewMatrix();
	void updateProjectionMatrix();
	void updateViewProjectionMatrix();

	mat4 getViewProjectionMatrix();
};