#include "transform.h"
#include <math.h>

// Converts a transform into a mat4
mat4 transformToMat4(const Transform& t) {
	// First, get the rotation basis of the transform
	vec3 x = t.rotation * vec3(1, 0, 0);
	vec3 y = t.rotation * vec3(0, 1, 0);
	vec3 z = t.rotation * vec3(0, 0, 1);
	// Next, scale the basis vectors
	x = x * t.scale.x;
	y = y * t.scale.y;
	z = z * t.scale.z;
	// Get the position of the transform
	vec3 p = t.position;
	// Create matrix
	return mat4(
		x.x, x.y, x.z, 0, // X basis (& Scale)
		y.x, y.y, y.z, 0, // Y basis (& scale)
		z.x, z.y, z.z, 0, // Z basis (& scale)
		p.x, p.y, p.z, 1  // Position
	);
}

// Extract the rotation and the translition from a matrix is easy. But not for the scale
// M = SRT, ignore the translation: M = SR -> invert R to isolate S
Transform mat4ToTransform(const mat4& m) {
	Transform out;
	out.position = vec3(m.v[12], m.v[13], m.v[14]);
	out.rotation = mat4ToQuat(m);
	mat4 rotScaleMat(
		m.v[0], m.v[1], m.v[2], 0,
		m.v[4], m.v[5], m.v[6], 0,
		m.v[8], m.v[9], m.v[10], 0,
		0, 0, 0, 1
	);
	mat4 invRotMat = quatToMat4(inverse(out.rotation));
	mat4 scaleSkewMat = rotScaleMat * invRotMat;
	out.scale = vec3(scaleSkewMat.v[0],
		scaleSkewMat.v[5],
		scaleSkewMat.v[10]
	);
	return out;
}

// Transforms can be combined in the same way as matrices and quaternions and the effects of two transforms can be combined into one transform
// To keep things consistent, combining transforms should maintain a right-to-left combination order
Transform combine(const Transform& t1, const Transform& t2) {
	Transform out;
	out.scale = t1.scale * t2.scale;
	out.rotation = t2.rotation * t1.rotation;// right-to-left multiplication (right is the first rotation applyed)
	// The combined position needs to be affected by the rotation and scale
	out.position = t1.rotation * (t1.scale * t2.position); // M = R*S*T
	out.position = t1.position + out.position;
	return out;
}

Transform inverse(const Transform& t) {
	Transform inv;
	inv.rotation = inverse(t.rotation);
	inv.scale.x = fabs(t.scale.x) < VEC3_EPSILON ?
		0.0f : 1.0f / t.scale.x;
	inv.scale.y = fabs(t.scale.y) < VEC3_EPSILON ?
		0.0f : 1.0f / t.scale.y;
	inv.scale.z = fabs(t.scale.z) < VEC3_EPSILON ?
		0.0f : 1.0f / t.scale.z;
	vec3 invTrans = t.position * -1.0f;
	inv.position = inv.rotation * (inv.scale * invTrans);
	return inv;
}

// First, apply the scale, then rotation
vec3 transformVector(const Transform& t, const vec3& v) {
	vec3 out;
	out = t.rotation * (t.scale * v);
	return out;
}