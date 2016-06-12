#include "CTM.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

GLuint CTM::MatricesUniBuffer;

void CTM::PushMatrix()
{
	auto mat = _model;
	_stack.push(mat);
}

void CTM::PopMatrix()
{
	_model = _stack.top();
	_stack.pop();
}

void CTM::LoadIdentity()
{
	_model = glm::mat4();
}

void CTM::SetModel() const
{
	glBindBuffer(GL_UNIFORM_BUFFER, MatricesUniBuffer);
	glBufferSubData(GL_UNIFORM_BUFFER, ModelMatrixOffset, MatrixSize, glm::value_ptr(_model));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void CTM::SetView(const glm::mat4& view) const
{
	glBindBuffer(GL_UNIFORM_BUFFER, MatricesUniBuffer);
	glBufferSubData(GL_UNIFORM_BUFFER, ViewMatrixOffset, MatrixSize, glm::value_ptr(view));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void CTM::SetOrthographic(const GLfloat& left, const GLfloat& right, const GLfloat& bottom, const GLfloat& top, const GLfloat& near, const GLfloat& far) const
{
	const auto orthographicProjection = glm::ortho(left, right, bottom, top, near, far);
	glBindBuffer(GL_UNIFORM_BUFFER, MatricesUniBuffer);
	glBufferSubData(GL_UNIFORM_BUFFER, ProjMatrixOffset, MatrixSize, glm::value_ptr(orthographicProjection));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void CTM::SetPerspective(const GLfloat& FOV, const GLfloat& aspectRatio, const GLfloat& nearPlane, const GLfloat& farPlane) const
{
	const auto perspectiveProjection = glm::perspective(glm::radians(FOV), aspectRatio, nearPlane, farPlane);
	glBindBuffer(GL_UNIFORM_BUFFER, MatricesUniBuffer);
	glBufferSubData(GL_UNIFORM_BUFFER, ProjMatrixOffset, MatrixSize, glm::value_ptr(perspectiveProjection));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void CTM::Translate(const glm::vec3& translate)
{
	_model = glm::translate(_model, translate);
}

void CTM::Translate(const GLfloat& x, const GLfloat& y, const GLfloat& z)
{
	_model = glm::translate(_model, glm::vec3(x, y, z));
}

void CTM::Scale(const glm::vec3& scale)
{
	_model = glm::scale(_model, scale);
}

void CTM::Scale(const GLfloat& x, const GLfloat& y, const GLfloat& z)
{
	_model = glm::scale(_model, glm::vec3(x, y, z));
}

void CTM::Rotate(const GLfloat& angle, const glm::vec3& axis)
{
	_model = glm::rotate(_model, glm::radians(angle), axis);
}

void CTM::Rotate(const GLfloat& angle, const GLfloat& x, const GLfloat& y, const GLfloat& z)
{
	_model = glm::rotate(_model, glm::radians(angle), glm::vec3(x, y, z));
}

void CTM::MultMatrix(const GLfloat mat[16])
{
	_model = _model * glm::make_mat4(mat);
}
