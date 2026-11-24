#pragma once

#include<glad/glad.h>
#include<stb/stb_image.h>
#include"shaderClass.h"

class Texture
{
public:
	GLuint ID; // ID de la textura
	const char* type; // Tipo de textura (por ejemplo, "diffuse", "specular", etc.)
	GLuint unit; // Unidad de textura (slot) en la que se encuentra la textura


	Texture(const char* image, const char* texType, GLuint slot, GLenum format, GLenum pixelType); // Constructor de la clase Texture

	void texUnit(Shader& shader, const char* uniform, GLuint unit); // Asigna la unidad de textura a un uniform en el shader

	void Bind();
	void Unbind();
	void Delete();

	
};
