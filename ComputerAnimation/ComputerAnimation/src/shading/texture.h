#pragma once
#include "../math/vec2.h"
#include "../math/vec3.h"
#include "../math/vec4.h"
#include "../math/quat.h"
class Texture {
protected:
	unsigned int mWidth;
	unsigned int mHeight;
	unsigned int mChannels;
	unsigned int mHandle;
private:
	Texture(const Texture& other);
public:
	Texture();
	Texture(const char* path);
	~Texture();
	Texture& operator=(const Texture& other);
	void Load(const char* path);
	void Load(const unsigned char* data, int width, int height, int channels);
	void Set(unsigned int uniform, unsigned int texIndex);
	void UnSet(unsigned int textureIndex);
	unsigned int GetHandle();
	ivec2 getSize();
};

class DataTexture {
protected:
	float* mData;
	unsigned int mSize;
	unsigned int mHandle;
public:
	DataTexture();
	DataTexture(const DataTexture&);
	DataTexture& operator=(const DataTexture&);
	~DataTexture();
	void Load(const char* path);
	void Save(const char* path);
	void UploadTextureDataToGPU(); 
	unsigned int Size();
	void Resize(unsigned int newSize);
	float* GetData();
	void SetTexel(unsigned int x, unsigned int y, const vec3& v);

	void Set(unsigned int uniform, unsigned int texture);
	void UnSet(unsigned int textureIndex);
	unsigned int GetHandle();
};