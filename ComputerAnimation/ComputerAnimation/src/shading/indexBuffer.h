#pragma once
#include <vector>
//A type of attribute. Unlike attributes, an index buffer is bound to GL_ELEMENT_ARRAY_BUFFER and can be used to draw primitives.
class IndexBuffer {
public:
	unsigned int mHandle; //OpenGL handle
	unsigned int mCount; //count of IndexBuffers
private:
	IndexBuffer(const IndexBuffer& other);
	IndexBuffer& operator=(const IndexBuffer& other);
public:
	IndexBuffer();
	~IndexBuffer();
	void Set(unsigned int* rr, unsigned int len);
	void Set(std::vector<unsigned int>& input);
	unsigned int Count();
	unsigned int GetHandle();
};
