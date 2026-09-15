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

	GLuint uniID = glGetUniformLocation(shaderProgram.ID, "scale");

	//textura
	Texture image("Bricks_001.png", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE);
	float rotation = 0.0f;
	double prevTime = glfwGetTime();


	GLuint tex0uni = glGetUniformLocation(shaderProgram.ID, "tex0"); //obtenemos la ubicacion de la textura en el shader)
	shaderProgram.Activate(); //activamos el shader)

	glm::mat4 model = glm::mat4(1.0f); //creamos una matriz de 4x4 para la transformacion del modelo
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 proj = glm::mat4(1.0f);

	model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(0.0f, 1.0f, 0.0f)); //rotamos el modelo 55 grados en el eje x
	view = glm::translate(view, glm::vec3(0.0f, -0.5f, -2.0f)); //movemos la camara hacia atras, realmente el mundo lo movemos hacia adelante
	proj = glm::perspective(glm::radians(45.0f), (float)(width / (float)height), 0.1f, 100.0f); //creamos una matriz de proyeccion perspectiva con un angulo de 45 grados, un aspect ratio de width/height, un near plane de 0.1 y un far plane de 100 para recortar la escena

	int modelLoc = glGetUniformLocation(shaderProgram.ID, "model"); //obtenemos la ubicacion de la matriz de transformacion del modelo en el shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); //asignamos la matriz de transformacion del modelo al shader

	int viewLoc = glGetUniformLocation(shaderProgram.ID, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	int projLoc = glGetUniformLocation(shaderProgram.ID, "proj");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

	glUniform1i(tex0uni, 0); //asignamos la textura al shader

	while (!glfwWindowShouldClose(window)) //ciclo para que no se cierre el codigo
	{
		glClearColor(0.02f, 0.10f, 0.30f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		shaderProgram.Activate(); //usamos el programa de shader)

		double crntTime = glfwGetTime();
		if (crntTime - prevTime >= 1.0 / 60.0)
		{
			prevTime = crntTime;
			rotation += 1.0f;
			model = glm::rotate(glm::mat4(1.0f), glm::radians(rotation - 55.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		}

		glUniform1f(uniID, 0.5f);
		image.Bind();
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