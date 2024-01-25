#pragma once
#include <iostream>
#include <map>


class Shader {
private:
	unsigned int mHandle;
	std::map<std::string, unsigned int> mAttributes;
	std::map<std::string, unsigned int> mUniforms;
private:
	Shader(const Shader&);
	Shader& operator=(const Shader&);
private:
	std::string ReadFile(const std::string& path);
	//returns an OpenGL handle.
	unsigned int CompileVertexShader(
		const std::string& vertex);
	//returns an OpenGL handle.
	unsigned int CompileFragmentShader(
		const std::string& fragment);
	//link two shaders into a shader program.
	bool LinkShaders(unsigned int vertex,
		unsigned int fragment);
	//fill out the attribute and uniform dictionaries
	void PopulateAttributes();
	void PopulateUniforms();
public:
	Shader();
	Shader(const std::string& vertex,
		const std::string& fragment);
	~Shader();
	
	//loads shaders from files and compiles them
	void Load(const std::string& vertex,
		const std::string& fragment);
	void Bind();
	void UnBind();
	unsigned int GetAttribute(const std::string& name);
	unsigned int GetUniform(const std::string& name);
	unsigned int GetHandle();
};