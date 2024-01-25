#include "shader.h"
#include <GL/glew.h>
#include <sstream>
#include <fstream>
#include <string>

//Abstracts the OpenGL calls
Shader::Shader() {
	mHandle = glCreateProgram();
}
Shader::Shader(const std::string& vertex,
	const std::string& fragment) {
	//create a new shader program handle
	mHandle = glCreateProgram();
	//loads shaders from files and compiles them
	Load(vertex, fragment);
}
Shader::~Shader() {
	glDeleteProgram(mHandle);
}

std::string Shader::ReadFile(const std::string& path) {
	std::ifstream file;
	file.open(path);
	std::stringstream contents;
	contents << file.rdbuf();
	file.close();
	return contents.str();
}

unsigned int Shader::CompileVertexShader(
	const std::string& vertex) {
	unsigned int v = glCreateShader(GL_VERTEX_SHADER);
	const char* v_source = vertex.c_str();
	glShaderSource(v, 1, &v_source, NULL);
	glCompileShader(v);
	int success = 0;
	glGetShaderiv(v, GL_COMPILE_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetShaderInfoLog(v, 512, NULL, infoLog);
		std::cout << "Vertex compilation failed.\n";
		std::cout << "\t" << infoLog << "\n";
		glDeleteShader(v);
		return 0;
	};
	return v;
}

unsigned int Shader::CompileFragmentShader(
	const std::string& fragment) {
	unsigned int f = glCreateShader(GL_FRAGMENT_SHADER);
	const char* f_source = fragment.c_str();
	glShaderSource(f, 1, &f_source, NULL);
	glCompileShader(f);
	int success = 0;
	glGetShaderiv(f, GL_COMPILE_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetShaderInfoLog(f, 512, NULL, infoLog);
		std::cout << "Fragment compilation failed.\n";
		std::cout << "\t" << infoLog << "\n";
		glDeleteShader(f);
		return 0;
	};
	return f;
}

bool Shader::LinkShaders(unsigned int vertex,
	unsigned int fragment) {
	glAttachShader(mHandle, vertex);
	glAttachShader(mHandle, fragment);
	glLinkProgram(mHandle);
	int success = 0;
	glGetProgramiv(mHandle, GL_LINK_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetProgramInfoLog(mHandle, 512, NULL, infoLog);
		std::cout << "ERROR: Shader linking failed.\n";
		std::cout << "\t" << infoLog << "\n";
		glDeleteShader(vertex);
		glDeleteShader(fragment);
		return false;
	}
	glDeleteShader(vertex);
	glDeleteShader(fragment);
	return true;
}

void Shader::PopulateAttributes() {
	int count = -1;
	int length;
	char name[128];
	int size;
	GLenum type;
	glUseProgram(mHandle);
	glGetProgramiv(mHandle, GL_ACTIVE_ATTRIBUTES,
		&count);
	for (int i = 0; i < count; ++i) {
		memset(name, 0, sizeof(char) * 128);
		glGetActiveAttrib(mHandle, (GLuint)i, 128,
			&length, &size, &type, name);
		int attrib = glGetAttribLocation(mHandle, name);
		if (attrib >= 0) {
			mAttributes[name] = attrib;
		}
	}
	glUseProgram(0);
}

void Shader::PopulateUniforms() {
	int count = -1;
	int length;
	char name[128];
	int size;
	GLenum type;
	char testName[256];
	glUseProgram(mHandle);
	glGetProgramiv(mHandle, GL_ACTIVE_UNIFORMS, &count);
	for (int i = 0; i < count; ++i) {
		memset(name, 0, sizeof(char) * 128);
		glGetActiveUniform(mHandle, (GLuint)i, 128,
			&length, &size, &type, name);
		int uniform = glGetUniformLocation(mHandle, name);
		if (uniform >= 0) { // Is uniform valid?
			std::string uniformName = name;
			// if name contains [, uniform is array
			std::size_t found = uniformName.find('[');
			if (found != std::string::npos) {
				uniformName.erase(uniformName.begin() +
					found, uniformName.end());
				unsigned int uniformIndex = 0;
				while (true) {
					memset(testName, 0, sizeof(char) * 256);
					sprintf_s(testName, "%s[%d]",
						uniformName.c_str(),
						uniformIndex++);
					int uniformLocation =
						glGetUniformLocation(
							mHandle, testName);
					if (uniformLocation < 0) {
						break;
					}
					mUniforms[testName] = uniformLocation;
				}
			}
			mUniforms[uniformName] = uniform;
		}
	}
	glUseProgram(0);
}

void Shader::Load(const std::string& vertex,
	const std::string& fragment) {
	std::ifstream f(vertex.c_str());
	bool vertFile = f.good();
	f.close();
	f = std::ifstream(vertex.c_str());
	bool fragFile = f.good();
	f.close();
	std::string v_source = vertex;
	if (vertFile) {
		v_source = ReadFile(vertex);
	}
	std::string f_source = fragment;
	if (fragFile) {
		f_source = ReadFile(fragment);
	}
	unsigned int vert = CompileVertexShader(v_source);
	unsigned int frag = CompileFragmentShader(f_source);
	if (LinkShaders(vert, frag)) {
		PopulateAttributes();
		PopulateUniforms();
	}
}

void Shader::Bind() {
	glUseProgram(mHandle);
}
void Shader::UnBind() {
	glUseProgram(0);
}
unsigned int Shader::GetHandle() {
	return mHandle;
}

unsigned int Shader::GetAttribute(
	const std::string& name) {
	std::map<std::string, unsigned int>::iterator it =
		mAttributes.find(name);
	if (it == mAttributes.end()) {
		std::cout << "Bad attrib index: " << name << "\n";
		return 0;
	}
	return it->second;
}

unsigned int Shader::GetUniform(const std::string& name) {
	std::map<std::string, unsigned int>::iterator it =
		mUniforms.find(name);
	if (it == mUniforms.end()) {
		std::cout << "Bad uniform index: " << name << "\n";
		return 0;
	}
	return it->second;
}