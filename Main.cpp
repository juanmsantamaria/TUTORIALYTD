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
	-1.0f, 0.0f,  1.0f,		0.0f, 0.0f, 0.0f,		0.0f, 0.0f,		0.0f, 1.0f, 0.0f,
	-1.0f, 0.0f, -1.0f,		0.0f, 0.0f, 0.0f,		0.0f, 1.0f,		0.0f, 1.0f, 0.0f,
	 1.0f, 0.0f, -1.0f,		0.0f, 0.0f, 0.0f,		1.0f, 1.0f,		0.0f, 1.0f, 0.0f,
	 1.0f, 0.0f,  1.0f,		0.0f, 0.0f, 0.0f,		1.0f, 0.0f,		0.0f, 1.0f, 0.0f
	};

	GLuint indices[] = //indices para dibujar el triangulo
	{
	0, 1, 2,
	0, 2, 3
	};

	GLfloat lightVertices[] = //vertices para la luz
	{
		-0.1f, -0.1f,  0.1f,
		-0.1f, -0.1f, -0.1f,
		 0.1f, -0.1f, -0.1f,
		 0.1f, -0.1f,  0.1f,
		-0.1f,  0.1f,  0.1f,
		-0.1f,  0.1f, -0.1f,
		 0.1f,  0.1f, -0.1f,
		 0.1f,  0.1f,  0.1f
	};

	GLuint lightIndices[] = //indices para dibujar la luz
	{
		0, 1, 2,
		0, 2, 3,
		0, 1, 5,
		0, 4, 5,
		1, 2, 6,
		1, 5, 6,
		2, 3, 7,
		2, 6, 7,
		3, 0, 4,
		3, 4, 7,
		4, 5, 6,
		4, 6, 7
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

	VAO1.LinkAttrib(VBO1, 0, 3, GL_FLOAT, 11 * sizeof(float), (void*)0); //ubicamos los datos del VBO, definiendo posicion y color
	VAO1.LinkAttrib(VBO1, 1, 3, GL_FLOAT, 11 * sizeof(float), (void*)(3 * sizeof(float))); //asignamos el buffer de vertices al VAO
	VAO1.LinkAttrib(VBO1, 2, 2, GL_FLOAT, 11 * sizeof(float), (void*)(6 * sizeof(float))); //asignamos las coordenadas de textura al VAO
	VAO1.LinkAttrib(VBO1, 3, 3, GL_FLOAT, 11 * sizeof(float), (void*)(8 * sizeof(float))); //asignamos las normales al VAO

	VAO1.Unbind(); // 1. Desvincular el VAO primero para "cerrar" el estado
	VBO1.Unbind(); // 2. Desvincular VBO
	EBO1.Unbind(); // 3. desvincular EBO

	Shader lightShader("light.vert", "light.frag"); //creamos un objeto de la clase shader y le pasamos los archivos de shader de vertice y fragmento para la luz

	VAO lightVAO; //creamos un objeto de la clase VAO para almacenar los datos de los vertices de la luz
	lightVAO.Bind(); //asignamos el buffer de vertices de la luz

	VBO lightVBO(lightVertices, sizeof(lightVertices)); //creamos un objeto de la clase VBO para almacenar los datos de los vertices de la luz
	EBO lightEBO(lightIndices, sizeof(lightIndices)); //creamos un objeto de la

	lightVAO.LinkAttrib(lightVBO, 0, 3, GL_FLOAT, 3 * sizeof(float), (void*)0); //ubicamos los datos del VBO de la luz, definiendo posicion

	lightVAO.Unbind(); // 1. Desvincular el VAO primero para "cerrar" el estado
	lightVBO.Unbind(); // 2. Desvincular VBO
	lightEBO.Unbind(); // 3. desvincular EBO

	glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //color de la luz

	glm::vec3 lightPos = glm::vec3(0.8f, 0.4f, 0.0f); //posicion de la luz
	glm::mat4 lightModel = glm::mat4(1.0f); //matriz de modelo de la luz
	lightModel = glm::translate(lightModel, lightPos); //trasladamos la luz

	glm::vec3 pyramidPos = glm::vec3(0.0f, 0.0f, 0.0f); //posicion de la piramide
	glm::mat4 pyramidModel = glm::mat4(1.0f); //matriz de modelo de la piramide
	pyramidModel = glm::translate(pyramidModel, pyramidPos); //trasladamos la

	lightShader.Activate(); //activamos el shader de la luz
	glUniformMatrix4fv(glGetUniformLocation(lightShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(lightModel));
	glUniform4f(glGetUniformLocation(lightShader.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, 1.0f); //asignamos el color de la luz al shader)
	shaderProgram.Activate(); //activamos el shader de la piramide
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "model"), 1, GL_FALSE, glm::value_ptr(pyramidModel));
	glUniform4f(glGetUniformLocation(shaderProgram.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, 1.0f); //asignamos el color de la luz al shader)
	glUniform3f(glGetUniformLocation(shaderProgram.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z); //asignamos la posicion de la luz al shader)

	//textura
	Texture image("wood2_COLOR.jpg", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGB, GL_UNSIGNED_BYTE);
	image.texUnit(shaderProgram, "tex0", 0);

	Texture wood2_SPEC("wood2_SPEC.jpg", GL_TEXTURE_2D, GL_TEXTURE1, GL_RED, GL_UNSIGNED_BYTE);
	wood2_SPEC.texUnit(shaderProgram, "tex1", 1);



	glEnable(GL_DEPTH_TEST); //habilitamos el test de profundidad para que no se vea la parte trasera del triangulo

	Camera camera(width, height, glm::vec3(0.0f, 0.0f, 2.0f)); //creamos un objeto de la clase camera y le pasamos la posicion de la camara



	shaderProgram.Activate(); //activamos el shader)


	glUniform3f(glGetUniformLocation(shaderProgram.ID, "camPos"), camera.Posicion.x, camera.Posicion.y, camera.Posicion.z); //asignamos la posicion de la camara al shader





	while (!glfwWindowShouldClose(window)) //ciclo para que no se cierre el codigo
	{
		glClearColor(0.02f, 0.10f, 0.30f, 1.0f); //asignamos un color de fondo a la ventana
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  //limpiamos el buffer de color y profundidad para que no se vea la parte trasera del triangulo
		shaderProgram.Activate(); //usamos el programa de shader)

		camera.Inputs(window); //funcion para mover la camara con las teclas WASD


		camera.updateMatrix(45.0f, 0.1f, 100.0f, shaderProgram, "camMatrix");
		glUniform3f(glGetUniformLocation(shaderProgram.ID, "camPos"), camera.Posicion.x, camera.Posicion.y, camera.Posicion.z); //actualizamos la posicion de la camara en el shader cada frame
		glActiveTexture(GL_TEXTURE0); //activamos la unidad de textura 0 para la textura de color
		image.Bind(); //asignamos la textura al shader
		glActiveTexture(GL_TEXTURE1); //activamos la unidad de textura 1 para la textura specular
		wood2_SPEC.Bind(); //asignamos la textura al shader
		VAO1.Bind(); //asignamos el buffer de vertices)
		glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(int), GL_UNSIGNED_INT, 0); //dibujamos el triangulo)

		lightShader.Activate(); //activamos el shader de la luz
		camera.updateMatrix(45.0f, 0.1f, 100.0f, lightShader, "camMatrix");
		lightVAO.Bind(); //asignamos el buffer de vertices de la luz
		glDrawElements(GL_TRIANGLES, sizeof(lightIndices) / sizeof(int), GL_UNSIGNED_INT, 0); //dibujamos la luz)



		glfwSwapBuffers(window); //intercambiamos el buffer frontal con el trasero para ver el color designado
		glfwPollEvents(); //funcion para que la ventana no se cierre y pueda recibir eventos



	}

	VAO1.Delete(); //eliminamos el VAO
	EBO1.Delete(); //eliminamos el EBO
	VBO1.Delete(); //eliminamos el VBO

	image.Delete(); //eliminamos la textura
	wood2_SPEC.Delete(); //eliminamos la textura specular

	shaderProgram.Delete(); //eliminamos el shader program


	glfwDestroyWindow(window);//funcion para destruir ventana
	glfwTerminate();
	return 0;
}