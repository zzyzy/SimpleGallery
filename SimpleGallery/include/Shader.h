#pragma once
#ifndef SHADER_H_INCLUDED
#define SHADER_H_INCLUDED

#include <iostream>

#include <GL/glew.h>

class Shader
{
public:
	Shader() = default;
	~Shader() = default;

	explicit Shader(const char* path);

	void Setup(const char* path);
	void Setup(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr);

	void Use() const;

	const GLuint& operator()() const;

private:
	const std::string _vertexShaderExt = ".vert";
	const std::string _fragmentShaderExt = ".frag";
	GLuint _program;

	static void checkCompileErrors(const GLuint& shader, const std::string& type);
};

#endif
