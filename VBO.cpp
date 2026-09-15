#include"VBO.h"

// constructor que genera un Vertex Buffer Object y lo vincula a los vértices
VBO::VBO(GLfloat* vertices, GLsizeiptr size)
{
	glGenBuffers(1, &ID);
	glBindBuffer(GL_ARRAY_BUFFER, ID);
	glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
}

// vincula este VBO en la tarjeta gráfica
void VBO::Bind()
{
	glBindBuffer(GL_ARRAY_BUFFER, ID);
}

// desvincula el VBO actual
void VBO::Unbind()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// libera memoria
void VBO::Delete()
{
	glDeleteBuffers(1, &ID);
}