#include"Texture.h"


// Constructor de la clase Texture
// Recibe la imagen, el tipo de textura, el slot, el formato y el tipo de píxel
Texture::Texture(const char* image, const char* texType, GLuint slot, GLenum format, GLenum pixelType)
{
	// Guardamos el tipo de textura
	type = texType;

	// Variables para almacenar el ancho, alto y número de canales de la imagen
	int widthImg, heightImg, numColCh;

	// Invertimos verticalmente la imagen para que se muestre correctamente en OpenGL
	stbi_set_flip_vertically_on_load(true);

	// Cargamos la imagen utilizando stb_image
	// El último parámetro 4 hace que la imagen se cargue siempre con 4 canales (RGBA)
	unsigned char* bytes = stbi_load(image, &widthImg, &heightImg, &numColCh, 4);


	// Generamos un identificador para la textura
	glGenTextures(1, &ID);

	// Activamos el slot de textura indicado
	glActiveTexture(GL_TEXTURE0 + slot);
	unit = slot;

	// Vinculamos la textura para poder configurarla
	glBindTexture(GL_TEXTURE_2D, ID);

	// Definimos el filtro utilizado cuando la textura se reduce
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	// Definimos el filtro utilizado cuando la textura aumenta
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Indicamos que la textura se repita horizontalmente
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

	// Indicamos que la textura se repita verticalmente
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Enviamos los datos de la imagen a OpenGL
	// GL_RGBA indica que la textura utiliza cuatro canales de color
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthImg, heightImg, 0, GL_RGBA, pixelType, bytes);

	// Generamos los mipmaps de la textura
	glGenerateMipmap(GL_TEXTURE_2D);

	// Liberamos la memoria utilizada por la imagen
	stbi_image_free(bytes);

	// Desvinculamos la textura
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::texUnit(Shader& shader, const char* uniform, GLuint unit)
{
	// Gets the location of the uniform
	GLuint texUni = glGetUniformLocation(shader.ID, uniform);
	// Shader needs to be activated before changing the value of a uniform
	shader.Activate();
	// Sets the value of the uniform
	glUniform1i(texUni, unit);
}


// Vincula la textura para poder utilizarla
void Texture::Bind()
{
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, ID);
}


// Desvincula la textura
void Texture::Unbind()
{
	glBindTexture(GL_TEXTURE_2D, 0);
}


// Elimina la textura de la memoria de OpenGL
void Texture::Delete()
{
	glDeleteTextures(1, &ID);
}