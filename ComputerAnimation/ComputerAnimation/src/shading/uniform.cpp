#include "uniform.h"
#include "../math/vec2.h"
#include "../math/vec3.h"
#include "../math/vec4.h"
#include "../math/quat.h"
#include "../math/mat4.h"
#include <GL/glew.h>

//supported uniform types
template Uniform<int>;
template Uniform<ivec4>;
template Uniform<ivec2>;
template Uniform<float>;
template Uniform<vec2>;
template Uniform<vec3>;
template Uniform<vec4>;
template Uniform<quat>;
template Uniform<mat4>;

//set a single unifrom
template <typename T>
void Uniform<T>::Set(unsigned int slot, const T& value) {
	Set(slot, (T*)&value, 1);
}
//set a vector
template <typename T>
void Uniform<T>::Set(unsigned int s, std::vector<T>& v) {
	Set(s, &v[0], (unsigned int)v.size());
}

//arguments: OpenGL function to call, type of structure, data type of the structure.
#define UNIFORM_IMPL(gl_func, tType, dType) \
template<> void Uniform<tType>::Set(unsigned int slot,\
tType* data, unsigned int length) {\
gl_func(slot, (GLsizei)length, (dType*)&data[0]); \
}

//generate setters for each type of data
UNIFORM_IMPL(glUniform1iv, int, int)
UNIFORM_IMPL(glUniform4iv, ivec4, int)
UNIFORM_IMPL(glUniform2iv, ivec2, int)
UNIFORM_IMPL(glUniform1fv, float, float)
UNIFORM_IMPL(glUniform2fv, vec2, float)
UNIFORM_IMPL(glUniform3fv, vec3, float)
UNIFORM_IMPL(glUniform4fv, vec4, float)
UNIFORM_IMPL(glUniform4fv, quat, float)

//generate setter for matrices. It takes an additional Boolean argument asking whether the matrix should be transposed or not.
template<> void Uniform<mat4>::Set(unsigned int slot,
	mat4* inputArray, unsigned int arrayLength) {
	glUniformMatrix4fv(slot, (GLsizei)arrayLength,
		false, (float*)&inputArray[0]);
}