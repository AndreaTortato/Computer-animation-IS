#include "Texture.h"
#include <GL/glew.h>
#include "../external/stb_image.h"

#include <fstream>
#include <iostream>
Texture::Texture() {
	mWidth = 0;
	mHeight = 0;
	mChannels = 0;
	glGenTextures(1, &mHandle);
}
Texture::Texture(const char* path) {
	glGenTextures(1, &mHandle);
	Load(path);
}

Texture::~Texture() {
	glDeleteTextures(1, &mHandle);
}
Texture& Texture::operator=(const Texture &texture) {
	mWidth = texture.mWidth;
	mHeight = texture.mHeight;
	mChannels = texture.mChannels;
	mHandle = texture.mHandle;
	return *this;
}

void Texture::Load(const char* path) {
	glBindTexture(GL_TEXTURE_2D, mHandle);
	int width, height, channels;
	unsigned char* data = stbi_load(path, &width, &height, &channels, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	mWidth = width;
	mHeight = height;
	mChannels = channels;
}

void Texture::Load(const unsigned char* data, int width, int height, int channels ) {
	glBindTexture(GL_TEXTURE_2D, mHandle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	mWidth = width;
	mHeight = height;
	mChannels = channels;
}

void Texture::Set(unsigned int uniformIndex, unsigned int textureIndex) {
	glActiveTexture(GL_TEXTURE0 + textureIndex);
	glBindTexture(GL_TEXTURE_2D, mHandle);
	glUniform1i(uniformIndex, textureIndex);
}

void Texture::UnSet(unsigned int textureIndex) {
	glActiveTexture(GL_TEXTURE0 + textureIndex);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
}

unsigned int Texture::GetHandle() {
	return mHandle;
}

ivec2 Texture::getSize() {
	return ivec2(mWidth, mHeight);
}

// ------------------------- DATA TEXTURE ---------------------------/
DataTexture::DataTexture() {
	mData = 0;
	mSize = 0;
	glGenTextures(1, &mHandle);
}

DataTexture::DataTexture(const DataTexture& other) {
	mData = 0;
	mSize = 0;
	glGenTextures(1, &mHandle);
	*this = other;
}

DataTexture& DataTexture::operator=(
	const DataTexture& other) {
	if (this == &other) {
		return *this;
	}
	mSize = other.mSize;
	if (mData != 0) {
		delete[] mData;
	}

	mData = 0;
	if (mSize != 0) {
		mData = new float[mSize * mSize * 4];
		memcpy(mData, other.mData,
			sizeof(float) * (mSize * mSize * 4));
	}
	return *this;
}

DataTexture::~DataTexture() {
	if (mData != 0) {
		delete[] mData;
	}
	glDeleteTextures(1, &mHandle);
}

void DataTexture::Save(const char* path) {
	std::ofstream file;
	file.open(path, std::ios::out | std::ios::binary);
	if (!file.is_open()) {
		std::cout << "Couldn't open " << path << "\n";
	}
	file << mSize;
	if (mSize != 0) {
		file.write((char*)mData,
			sizeof(float) * (mSize * mSize * 4));
	}
	file.close();
}

void DataTexture::Load(const char* path) {
	std::ifstream file;
	file.open(path, std::ios::in | std::ios::binary);
	if (!file.is_open()) {
		std::cout << "Couldn't open " << path << "\n";
	}
	file >> mSize;
	mData = new float[mSize * mSize * 4];
	file.read((char*)mData,
		sizeof(float) * (mSize * mSize * 4));
	file.close();
	UploadTextureDataToGPU();
}

void DataTexture::UploadTextureDataToGPU() {
	glBindTexture(GL_TEXTURE_2D, mHandle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, mSize, mSize, 0, GL_RGBA, GL_FLOAT, mData);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
}

unsigned int DataTexture::Size() {
	return mSize;
}
unsigned int DataTexture::GetHandle() {
	return mHandle;
}
float* DataTexture::GetData() {
	return mData;
}
void DataTexture::Resize(unsigned int newSize) {
	if (mData != 0) {
		delete[] mData;
	}
	mSize = newSize;
	mData = new float[mSize * mSize * 4];
}

void DataTexture::Set(unsigned int uniformIndex, unsigned int textureIndex) {
	glActiveTexture(GL_TEXTURE0 + textureIndex);
	glBindTexture(GL_TEXTURE_2D, mHandle);
	glUniform1i(uniformIndex, textureIndex);
}

void DataTexture::UnSet(unsigned int textureIndex) {
	glActiveTexture(GL_TEXTURE0 + textureIndex);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
}

void DataTexture::SetTexel(unsigned int x, unsigned int y, const vec3& v) {
	// [CA] To do: Compute the current index where to start saving the data into the array mData. Assume the data has 4 channels (RGBA). 
	// Find the index of the array given the x and y coordinates of the texture and the mSize (width of the texture, it assumes that its squared)
	unsigned int index = 4 * (x * mSize + y); 
	
	// Set the data into the mData. Put a 0 in the last channel.
	mData[index] = v.x;
	mData[index + 1] = v.y;
	mData[index + 2] = v.z;
	mData[index + 3] = 0;
}
