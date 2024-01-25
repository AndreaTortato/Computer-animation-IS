#pragma once
#include "indexBuffer.h"
enum class DrawMode {
	Points,
	LineStrip,
	LineLoop,
	Lines,
	Triangles,
	TriangleStrip,
	TriangleFan
};

void Draw(IndexBuffer& inIndexBuffer, DrawMode mode);
void Draw(unsigned int vertexCount, DrawMode mode);

//instanceCount: how many instances of the geometry will be rendered
void DrawInstanced(IndexBuffer& inIndexBuffer,
	DrawMode mode, unsigned int instanceCount);
void DrawInstanced(unsigned int vertexCount,
	DrawMode mode, unsigned int numInstances);