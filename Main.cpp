#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<stb/stb_image.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
//incluimos librerias de OpenGL y GLFW para poder crear una ventana y renderizar gráficos en ella.

#include"shaderClass.h"
#include"VAO.h"
#include"EBO.h"
#include"Texture.h"
#include"CAMERA.h"




int main()
{
	const unsigned int width = 800;
	const unsigned int height = 800;
	//inicializamos GLFW y creamos una ventana de 800x800 píxeles con el título "Youtube"
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); //decimos que version de open gl usamos
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); //version menor
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //introducimos profile, un paquete de funciones. perfil core para usar funciones modernas.

	GLfloat vertices[] = //usamos el float de open gl para designar la ubicacion de los vertices
	{
		//    posición              color                 UV
			-0.5f, 0.0f,  0.5f,     0.83f, 0.70f, 0.44f,	0.0f, 0.0f,
			-0.5f, 0.0f, -0.5f,     0.83f, 0.70f, 0.44f,	5.0f, 0.0f,
			 0.5f, 0.0f, -0.5f,     0.83f, 0.70f, 0.44f,	0.0f, 0.0f,
			 0.5f, 0.0f,  0.5f,     0.83f, 0.70f, 0.44f,	5.0f, 0.0f,
			 0.0f, 0.8f,  0.0f,     0.92f, 0.86f, 0.76f,	2.5f, 5.0f
	};

	GLuint indices[] = //indices para dibujar el triangulo
	{
	0, 1, 2,
	0, 2, 3,
	0, 1, 4,
	1, 2, 4,
	2, 3, 4,
	3, 0, 4
	};

	GLFWwindow* window = glfwCreateWindow(width, height, "Youtube", NULL, NULL); //especificaciones de la ventana (ancho, alto, título, pantalla completa )
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl; //error message 
		glfwTerminate(); //finalizamos el gfwl
		return -1;
	}
	glfwMakeContextCurrent(window); //reconoce la ventana como item 


	gladLoadGL(); //confirmamos que glad se ha cargado correctamente y podemos usar las funciones


	glViewport(0, 0, width, height); //limitar la ventana a 800x800

	Shader shaderProgram("default.vert", "default.frag"); //creamos un objeto de la clase shader y le pasamos los archivos de shader de vertice y fragmento

	VAO VAO1; //creamos un objeto de la clase VAO para almacenar los datos de los vertices
	VAO1.Bind(); //asignamos el buffer de vertices

	VBO VBO1(vertices, sizeof(vertices)); //creamos un objeto de la clase VBO para almacenar los datos de los vertices
	EBO EBO1(indices, sizeof(indices)); //creamos un objeto de la clase EBO para almacenar los datos de los indices

	VAO1.LinkAttrib(VBO1, 0, 3, GL_FLOAT, 8 * sizeof(float), (void*)0); //ubicamos los datos del VBO, definiendo posicion y color
	VAO1.LinkAttrib(VBO1, 1, 3, GL_FLOAT, 8 * sizeof(float), (void*)(3 * sizeof(float))); //asignamos el buffer de vertices al VAO
	VAO1.LinkAttrib(VBO1, 2, 2, GL_FLOAT, 8 * sizeof(float), (void*)(6 * sizeof(float))); //asignamos las coordenadas de textura al VAO

	VAO1.Unbind(); // 1. Desvincular el VAO primero para "cerrar" el estado
	VBO1.Unbind(); // 2. Desvincular VBO
	EBO1.Unbind(); // 3. desvincular EBO

	

	//textura
	Texture image("Bricks_001.png", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE); //creamos un objeto de la clase texture y le pasamos la ruta de la imagen, el tipo de textura, el slot de textura, el formato y el tipo de pixel

	glEnable(GL_DEPTH_TEST); //habilitamos el test de profundidad para que no se vea la parte trasera del triangulo

	Camera camera(width, height, glm::vec3(0.0f, 0.0f, 2.0f)); //creamos un objeto de la clase camera y le pasamos la posicion de la camara



	shaderProgram.Activate(); //activamos el shader)


	

	



	while (!glfwWindowShouldClose(window)) //ciclo para que no se cierre el codigo
	{
		glClearColor(0.02f, 0.10f, 0.30f, 1.0f); //asignamos un color de fondo a la ventana
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  //limpiamos el buffer de color y profundidad para que no se vea la parte trasera del triangulo
		shaderProgram.Activate(); //usamos el programa de shader)

		camera.Inputs(window); //funcion para mover la camara con las teclas WASD
		

		camera.Matrix(45.0f, 0.1f, 100.0f, shaderProgram, "camMatrix"); //actualizamos la matriz de proyeccion y vista de la camara
		image.Bind(); //asignamos la textura al shader
		VAO1.Bind(); //asignamos el buffer de vertices)
		glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(int), GL_UNSIGNED_INT, 0); //dibujamos el triangulo)
		glfwSwapBuffers(window); //intercambiamos el buffer frontal con el trasero para ver el color designado
		glfwPollEvents(); //funcion para que la ventana no se cierre y pueda recibir eventos



	}

	VAO1.Delete(); //eliminamos el VAO
	EBO1.Delete(); //eliminamos el EBO
	VBO1.Delete(); //eliminamos el VBO

	image.Delete(); //eliminamos la textura
	shaderProgram.Delete(); //eliminamos el shader program


	glfwDestroyWindow(window);//funcion para destruir ventana
	glfwTerminate();
	return 0;
}