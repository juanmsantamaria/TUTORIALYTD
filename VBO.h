#ifndef VBO_CLASS_H
#define VBO_CLASS_H


#include<glm/glm.hpp>
#include<vector>
#include<glad/glad.h>
// Constructor: recibe un puntero al arreglo de vértices y su tamaño


struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 color;
	glm::vec2 texUV;
};
class VBO
{
public:
	GLuint ID;
	VBO(std::vector<Vertex>& vertices);
	//vincula este VBO en la tarjeta gráfica
	void Bind();
	// desvincula el VBO actual
	void Unbind();
	//libera memoria
	void Delete();
	
};

#endif
