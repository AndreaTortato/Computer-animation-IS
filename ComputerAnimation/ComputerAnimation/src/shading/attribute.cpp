#include "attribute.h"
#include "../math/vec2.h"
#include "../math/vec3.h"
#include "../math/vec4.h"
#include <GL/glew.h>

template Attribute<int>;
template Attribute<float>;
template Attribute<vec2>;
template Attribute<vec3>;
template Attribute<vec4>;
template Attribute<ivec4>;

//generate an OpenGL buffer and store it
template<typename T>
Attribute<T>::Attribute() {
	glGenBuffers(1, &mHandle);
	mCount = 0;
}
template<typename T>
Attribute<T>::~Attribute() {
	glDeleteBuffers(1, &mHandle);
}

//retrieve total count of attributes
template<typename T>
unsigned int Attribute<T>::Count() {
	return mCount;
}
//retrieve the OpenGL hande
template<typename T>
unsigned int Attribute<T>::GetHandle() {
	return mHandle;
}

//fill the buffer with the data
template<typename T>
void Attribute<T>::Set(T* inputArray,
	unsigned int arrayLength) {
	mCount = arrayLength;
	unsigned int size = sizeof(T);
	glBindBuffer(GL_ARRAY_BUFFER, mHandle);
	glBufferData(GL_ARRAY_BUFFER, size * mCount,
		inputArray, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
template<typename T>
void Attribute<T>::Set(std::vector<T>& input) {
	Set(&input[0], (unsigned int)input.size());
}

template<>
void Attribute<int>::SetAttribPointer(unsigned int s) {
	glVertexAttribIPointer(s, 1, GL_INT, 0, (void*)0);
}
template<>
void Attribute<ivec4>::SetAttribPointer(unsigned int s) {
	glVertexAttribIPointer(s, 4, GL_INT, 0, (void*)0);
}
template<>
void Attribute<float>::SetAttribPointer(unsigned int s) {
	glVertexAttribPointer(s, 1, GL_FLOAT, GL_FALSE, 0, 0);
}

template<>
void Attribute<vec2>::SetAttribPointer(unsigned int s) {
	glVertexAttribPointer(s, 2, GL_FLOAT, GL_FALSE, 0, 0);
}
template<>
void Attribute<vec3>::SetAttribPointer(unsigned int s) {
	glVertexAttribPointer(s, 3, GL_FLOAT, GL_FALSE, 0, 0);
}
template<>
void Attribute<vec4>::SetAttribPointer(unsigned int s) {
	glVertexAttribPointer(s, 4, GL_FLOAT, GL_FALSE, 0, 0);
}

//bind the attribute to the slot specified in the Shader class
template<typename T>
void Attribute<T>::BindTo(unsigned int slot) {
	glBindBuffer(GL_ARRAY_BUFFER, mHandle);
	glEnableVertexAttribArray(slot);
	SetAttribPointer(slot);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

//unbind the attribute to the slot specified in the Shader class
template<typename T>
void Attribute<T>::UnBindFrom(unsigned int slot) {
	glBindBuffer(GL_ARRAY_BUFFER, mHandle);
	glDisableVertexAttribArray(slot);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}