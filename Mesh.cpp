#include "Mesh.h"

Mesh::Mesh(std::vector <Vertex>& vertices, std::vector <GLuint>& indices, std::vector <Texture>& textures)
{
	Mesh::vertices = vertices;
	Mesh::indices = indices;
	Mesh::textures = textures;

	VAO.Bind();
	// generamos vertices buffer object y lo vinculamos a los vertices
	VBO VBO1(vertices);
	// Generamos un elemento buffer object y lo vinculamos a los indices
	EBO EBO1(indices);
	// Links para que OpenGL sepa cómo interpretar los datos de los vértices
	VAO.LinkAttrib(VBO1, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)0);
	VAO.LinkAttrib(VBO1, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)(3 * sizeof(float)));
	VAO.LinkAttrib(VBO1, 2, 3, GL_FLOAT, sizeof(Vertex), (void*)(6 * sizeof(float)));
	VAO.LinkAttrib(VBO1, 3, 2, GL_FLOAT, sizeof(Vertex), (void*)(9 * sizeof(float)));
	// guardamos los ids de los buffers para poder actualizarlos despues
	vboID = VBO1.ID;
	eboID = EBO1.ID;
	// 
	VAO.Unbind();
	VBO1.Unbind();
	EBO1.Unbind();
}


void Mesh::Draw(Shader& shader, Camera& camera)
{
	// Bind shader to be able to access uniforms
	shader.Activate();
	VAO.Bind();

	// 
	unsigned int numDiffuse = 0;
	unsigned int numSpecular = 0;

	for (unsigned int i = 0; i < textures.size(); i++) // iteramos sobre todas las texturas de la malla
	{
		std::string num;
		std::string type = textures[i].type; // obtenemos el tipo de textura (diffuse, specular, etc.)
		if (type == "diffuse")
		{
			num = std::to_string(numDiffuse++); // si es una textura difusa, incrementamos el contador de texturas difusas
		}
		else if (type == "specular") // si es una textura especular, incrementamos el contador de texturas especulares
		{
			num = std::to_string(numSpecular++); // si es una textura especular, incrementamos el contador de texturas especulares
		}
		textures[i].texUnit(shader, (type + num).c_str(), i); // asignamos la unidad de textura al shader
		textures[i].Bind();
	}
	// Take care of the camera Matrix
	glUniform3f(glGetUniformLocation(shader.ID, "camPos"), camera.Posicion.x, camera.Posicion.y, camera.Posicion.z);
	// la camMatrix ya se envia desde main con camera.updateMatrix(...)

	// Draw the actual mesh
	glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
}


void Mesh::Update(std::vector <Vertex>& newVertices, std::vector <GLuint>& newIndices) // reemplaza los vertices e indices de la malla (se usa al construir o destruir bloques)
{
	vertices = newVertices;
	indices = newIndices;

	// con el VAO vinculado, el EBO queda asociado a el
	VAO.Bind();

	glBindBuffer(GL_ARRAY_BUFFER, vboID);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_DYNAMIC_DRAW); // actualizamos los vertices

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboID); // con el VAO vinculado, el EBO queda asociado a el
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_DYNAMIC_DRAW); // actualizamos los indices

	VAO.Unbind();
	glBindBuffer(GL_ARRAY_BUFFER, 0); // desvinculamos el VBO
}