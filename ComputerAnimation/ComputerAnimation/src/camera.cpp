#include "camera.h"
#include <iostream>
#include "math/transform.h"
Camera::Camera()
{
	setOrthographic(-1, 1, 1, -1, -1, 1);
}

vec3 Camera::getLocalVector(const vec3& v)
{
	mat4 iV = inverse(view_matrix);
	
	vec3 result = transformVector(iV, v);
	return result;
}

vec3 Camera::projectVector(vec3 pos, bool& negZ)
{
	vec4 pos4 = vec4(pos.x, pos.y, pos.z, 1.0);
	vec4 result = viewprojection_matrix * pos4;
	negZ = result.z < 0;
	if (type == ORTHOGRAPHIC)
		return vec3(result.x, result.y, result.z);
	else
		return vec3(result.x, result.y, result.z) / result.w;
}

void Camera::rotate(float angle, const vec3& axis)
{

	vec3 front = center - eye;
	//normalize(front);
	quat rotation = angleAxis(angle, axis);
	Transform t;
	t.rotation = rotation;

	vec3 newFront = transformVector(t, front);
	center = newFront + eye;
	updateViewMatrix();
}

void Camera::orbit(float angle, const vec3& axis)
{

	vec3 front = eye - center;
	//normalize(front);
	quat rotation = angleAxis(angle, axis);
	Transform t;
	t.rotation = rotation;

	vec3 newFront = transformVector(t, front);
	eye = newFront + center;
	updateViewMatrix();
}

void Camera::move(vec3 delta)
{
	vec3 localDelta = getLocalVector(delta);
	eye = eye - localDelta;
	center = center - localDelta;
	updateViewMatrix();
}

void Camera::setOrthographic(float left, float right, float top, float bottom, float near_plane, float far_plane)
{
	type = ORTHOGRAPHIC;

	this->left = left;
	this->right = right;
	this->top = top;
	this->bottom = bottom;
	this->near_plane = near_plane;
	this->far_plane = far_plane;

	updateProjectionMatrix();
}

void Camera::setPerspective(float fov, float aspect, float near_plane, float far_plane)
{
	type = PERSPECTIVE;

	this->fov = fov;
	this->aspect = aspect;
	this->near_plane = near_plane;
	this->far_plane = far_plane;

	updateProjectionMatrix();
}

void Camera::lookAt(const vec3& eye, const vec3& center, const vec3& up)
{
	this->eye = eye;
	this->center = center;
	this->up = up;

	updateViewMatrix();
}

void Camera::updateViewMatrix()
{
	// Reset Matrix (Identity)
	view_matrix = mat4();

	//Front/Forward vector
	vec3 f = normalized(center - eye) * -1.0f;

	//Right/Side vector
	vec3 r = cross(up, f); // Right handed
	if (r == vec3(0, 0, 0)) {
		//std::cout << "Error: View and up vector are parallels\n";
		return; // Error
	}
	normalize(r);

	//Uo/Top vector
	vec3 u = normalized(cross(f, r)); // Right handed
	vec3 t = vec3(
		-dot(r, eye),
		-dot(u, eye),
		-dot(f, eye)
	);
	view_matrix = mat4(
		// Transpose upper 3x3 matrix to invert it
		r.x, u.x, f.x, 0,
		r.y, u.y, f.y, 0,
		r.z, u.z, f.z, 0,
		t.x, t.y, t.z, 1
	);

	updateViewProjectionMatrix();
}

// Create a projection matrix
void Camera::updateProjectionMatrix()
{
	// Reset Matrix (Identity)
	projection_matrix = mat4();

	if (left == right || top == bottom || near_plane == far_plane) {
		std::cout << "Error: Invalid frustum\n";
		return; // Error
	}

	if (type == PERSPECTIVE) {
		float ymax = near_plane * tanf(fov * 3.14159265359f / 360.0f);
		float xmax = ymax * aspect;
		left = -xmax; right = xmax; bottom = -ymax; top = ymax;
		projection_matrix = mat4(
			(2.0f * near_plane) / (right - left), 0, 0, 0,
			0, (2.0f * near_plane) / (top - bottom), 0, 0,
			(right + left) / (right - left), (top + bottom) / (top - bottom), (-(far_plane + near_plane)) / (far_plane - near_plane), -1,
			0, 0, (-2 * far_plane * near_plane) / (far_plane - near_plane), 0
		);
	}
	else if (type == ORTHOGRAPHIC) {
		projection_matrix = mat4(
			2.0f / (right - left), 0, 0, 0,
			0, 2.0f / (top - bottom), 0, 0,
			0, 0, -2.0f / (far_plane - near_plane), 0,
			-((right + left) / (right - left)), -((top + bottom) / (top - bottom)), -((far_plane + near_plane) / (far_plane - near_plane)), 1
		);
	}

	updateViewProjectionMatrix();
}

void Camera::updateViewProjectionMatrix()
{
	viewprojection_matrix = projection_matrix * view_matrix;
}

mat4 Camera::getViewProjectionMatrix()
{
	updateViewMatrix();
	updateProjectionMatrix();

	return viewprojection_matrix;
}
