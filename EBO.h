#ifndef EBO_CLASS_H
#define EBO_CLASS_H

#include<glad/glad.h>
#include<vector>

class EBO
{
public:
	// referencia del objeto buffer
	GLuint ID;
	// constructor que genera un EBO y lo enlaza a los indices
	EBO(std::vector<GLuint>& indices);

	// borra EBO
	void Bind();
	// borra EBO
	void Unbind();
	// borra EBO
	void Delete();
};

#endif