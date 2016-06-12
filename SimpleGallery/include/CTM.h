#pragma once
#ifndef CTM_H_INCLUDED
#define CTM_H_INCLUDED

#include <stack>

#include <GL/glew.h>
#include <glm/glm.hpp>

#define MatricesUniBufferSize sizeof(float) * 16 * 3
#define ProjMatrixOffset 0
#define ViewMatrixOffset sizeof(float) * 16
#define ModelMatrixOffset sizeof(float) * 16 * 2
#define MatrixSize sizeof(float) * 16

class CTM
{
public:
	CTM() = default;
	~CTM() = default;

	static GLuint MatricesUniBuffer;

	void PushMatrix();
	void PopMatrix();

	void LoadIdentity();

	void SetModel() const;
	void SetView(const glm::mat4& view) const;
	void SetOrthographic(const GLfloat& left, const GLfloat& right, const GLfloat& bottom, const GLfloat& top, const GLfloat& near, const GLfloat& far) const;
	void SetPerspective(const GLfloat& FOV, const GLfloat& aspectRatio, const GLfloat& near, const GLfloat& far) const;

	void Translate(const glm::vec3& translate);
	void Translate(const GLfloat& x, const GLfloat& y, const GLfloat& z);
	void Scale(const glm::vec3& scale);
	void Scale(const GLfloat& x, const GLfloat& y, const GLfloat& z);
	void Rotate(const GLfloat& angle, const glm::vec3& axis);
	void Rotate(const GLfloat& angle, const GLfloat& x, const GLfloat& y, const GLfloat& z);

	void MultMatrix(const GLfloat mat[16]);

private:
	glm::mat4 _model;
	std::stack<glm::mat4> _stack;
};

#endif
