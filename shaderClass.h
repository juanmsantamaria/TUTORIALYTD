#ifndef SHADER_CLASS_H // Guarda de inclusión: evita que el archivo se compile múltiples veces
#define SHADER_CLASS_H


#include<glad/glad.h>
#include<string>
#include<fstream>
#include<sstream>
#include<iostream>
#include<cerrno>

// Función auxiliar de traduccion de archivos: lee el contenido de un archivo y lo devuelve como una cadena
std::string get_file_contents(const char* filename); // Funcion para leer el contenido de un archivo y traducirlo

class Shader
{
public:
	// Identificador numérico asignado por OpenGL al programa compilado en la GPU
	GLuint ID;



	// Constructor: Lee los archivos, compila los shaders y los enlaza en el ID
	Shader(const char* vertexfile, const char* fragmentfile);

	// Le indica a la GPU que use este shader para los siguientes objetos
	void Activate();
	//borra el programa de los shaders
	void Delete();
private:


	void compileErrors(unsigned int shader, const char* type);

};

#endif