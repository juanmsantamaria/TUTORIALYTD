#include"shaderClass.h"

// Función para leer el contenido de un archivo y devolverlo como una cadena
std::string get_file_contents(const char* filename)
{
	std::ifstream in(filename, std::ios::binary); // Abrimos el archivo en modo binario
	if (in) // Si el archivo se abrió correctamente
	{
		std::string contents; // Creamos una cadena para almacenar el contenido del archivo
		in.seekg(0, std::ios::end); // Movemos el cursor al final del archivo
		contents.resize(in.tellg()); // Redimensionamos la cadena para que tenga el mismo tamaño que el archivo
		in.seekg(0, std::ios::beg); // Movemos el cursor al inicio del archivo
		in.read(&contents[0], contents.size()); // Leemos el contenido del archivo y lo almacenamos en la cadena
		in.close();		// Cerramos el archivo
		return(contents);	// Devolvemos el contenido del archivo
	}
	throw(errno);
}

// Contructor de dos shaders
Shader::Shader(const char* vertexFile, const char* fragmentFile)
{
	// lee la informacion de la carpeta shaders
	std::string vertexCode = get_file_contents(vertexFile);
	std::string fragmentCode = get_file_contents(fragmentFile);

	// pasa la informacion de los shaders a arrays
	const char* vertexSource = vertexCode.c_str();
	const char* fragmentSource = fragmentCode.c_str();

	// crea un objeto vertice
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	// Attach Vertex Shader source to the Vertex Shader Object
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	// compila el shader
	glCompileShader(vertexShader);
	// revisa si compila
	compileErrors(vertexShader, "VERTEX");

	// crea un objeto fragmento
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	// Attach Fragment Shader source to the Fragment Shader Object
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	// compila el fragmento
	glCompileShader(fragmentShader);
	// revisa si compila 
	compileErrors(fragmentShader, "FRAGMENT");

	// crea un objeto del shader program
	ID = glCreateProgram();
	// Attach the Vertex and Fragment Shaders to the Shader Program
	glAttachShader(ID, vertexShader);
	glAttachShader(ID, fragmentShader);
	//
	glLinkProgram(ID);
	// revisa si compila
	compileErrors(ID, "PROGRAM");

	// Delete the now useless Vertex and Fragment Shader objects
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

}

// Activate el shader
void Shader::Activate()
{
	glUseProgram(ID);
}

// elimina
void Shader::Delete()
{
	glDeleteProgram(ID);
}

// revisamos con el rectificador de errores
void Shader::compileErrors(unsigned int shader, const char* type)
{
	// Stores status of compilation
	GLint hasCompiled;
	// error mesage 
	char infoLog[1024];
	if (type != "PROGRAM") // si no es un programa, es un shader
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &hasCompiled); // revisa si compila
		if (hasCompiled == GL_FALSE) // si no compila, imprime el error
		{
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);  // obtiene el error
			std::cout << "SHADER_COMPILATION_ERROR for:" << type << "\n" << infoLog << std::endl; // imprime el error
		}
	}
	else 
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &hasCompiled); // revisa si compila
		if (hasCompiled == GL_FALSE) // si no compila, imprime el error
		{
			glGetProgramInfoLog(shader, 1024, NULL, infoLog); // obtiene el error
			std::cout << "SHADER_LINKING_ERROR for:" << type << "\n" << infoLog << std::endl; // imprime el error
		}
	}
}