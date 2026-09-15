#include"Texture.h"


// Constructor de la clase Texture
// Recibe la imagen, el tipo de textura, el slot, el formato y el tipo de píxel
Texture::Texture(const char* image, GLenum texType, GLenum slot, GLenum format, GLenum pixelType)
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
	glActiveTexture(slot);

	// Vinculamos la textura para poder configurarla
	glBindTexture(texType, ID);

	// Definimos el filtro utilizado cuando la textura se reduce
	glTexParameteri(texType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Definimos el filtro utilizado cuando la textura aumenta
	glTexParameteri(texType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Indicamos que la textura se repita horizontalmente
	glTexParameteri(texType, GL_TEXTURE_WRAP_S, GL_REPEAT);

	// Indicamos que la textura se repita verticalmente
	glTexParameteri(texType, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Enviamos los datos de la imagen a OpenGL
	// GL_RGBA indica que la textura utiliza cuatro canales de color
	glTexImage2D(texType, 0, GL_RGBA, widthImg, heightImg, 0, GL_RGBA, pixelType, bytes);

	// Generamos los mipmaps de la textura
	glGenerateMipmap(texType);

	// Liberamos la memoria utilizada por la imagen
	stbi_image_free(bytes);

	// Desvinculamos la textura
	glBindTexture(texType, 0);
}


// Vincula la textura para poder utilizarla
void Texture::Bind()
{
	glBindTexture(type, ID);
}


// Desvincula la textura
void Texture::Unbind()
{
	glBindTexture(type, 0);
}


// Elimina la textura de la memoria de OpenGL
void Texture::Delete()
{
	glDeleteTextures(1, &ID);
}