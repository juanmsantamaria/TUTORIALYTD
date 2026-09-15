#pragma once

#include<glad/glad.h>
#include<stb/stb_image.h>

class Texture
{
public:
	GLuint ID;
	GLenum type;

	Texture(const char* image, GLenum texType, GLenum slot, GLenum format, GLenum pixelType);

	void Bind();
	void Unbind();
	void Delete();
};
