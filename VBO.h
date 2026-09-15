#ifndef VBO_CLASS_H
#define VBO_CLASS_H

#include<glad/glad.h>
// Constructor: recibe un puntero al arreglo de vértices y su tamaño
class VBO
{
public:
	GLuint ID;
	VBO(GLfloat* vertices, GLsizeiptr size);
	//vincula este VBO en la tarjeta gráfica
	void Bind();
	// desvincula el VBO actual
	void Unbind();
	//libera memoria
	void Delete();
	
};

#endif
