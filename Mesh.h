#ifndef MESH_CLASS_H
#define MESH_CLASS_H

#include<string>
#include<vector>

#include"VAO.h"
#include"EBO.h"
#include"Camera.h"
#include"Texture.h"

class Mesh
{
public:
	std::vector <Vertex> vertices;
	std::vector <GLuint> indices;
	std::vector <Texture> textures;
	// Store VAO in public so it can be used in the Draw function
	VAO VAO;

	// ids de los buffers, para poder actualizar la malla despues
	GLuint vboID;
	GLuint eboID;

	// Initializes the mesh
	Mesh(std::vector <Vertex>& vertices, std::vector <GLuint>& indices, std::vector <Texture>& textures);

	// Draws the mesh
	void Draw(Shader& shader, Camera& camera);

	// reemplaza los vertices e indices de la malla (se usa al construir o destruir bloques)
	void Update(std::vector <Vertex>& newVertices, std::vector <GLuint>& newIndices);
};
#endif