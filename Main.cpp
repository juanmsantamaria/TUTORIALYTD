#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<filesystem>
namespace fs = std::filesystem;
#include"Mesh.h"




int main()
{
	const unsigned int width = 800;
	const unsigned int height = 800;
	//inicializamos GLFW y creamos una ventana de 800x800 píxeles con el título "Youtube"
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); //decimos que version de open gl usamos
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); //version menor
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //introducimos profile, un paquete de funciones. perfil core para usar funciones modernas.

	Vertex vertices[] = //usamos el float de open gl para designar la ubicacion de los vertices

	{ //               COORDINATES           /            COLORS          /           NORMALS         /       TEXTURE COORDINATES    //
	Vertex{glm::vec3(-1.0f, 0.0f,  1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f)},
	Vertex{glm::vec3(-1.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 1.0f)},
	Vertex{glm::vec3(1.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 1.0f)},
	Vertex{glm::vec3(1.0f, 0.0f,  1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 0.0f)}
	};

	GLuint indices[] = //indices para dibujar el triangulo
	{
	0, 1, 2,
	0, 2, 3
	};

	Vertex lightVertices[] = //vertices para la luz
	{
	Vertex{glm::vec3(-0.1f, -0.1f,  0.1f)},
	Vertex{glm::vec3(-0.1f, -0.1f, -0.1f)},
	Vertex{glm::vec3(0.1f, -0.1f, -0.1f)},
	Vertex{glm::vec3(0.1f, -0.1f,  0.1f)},
	Vertex{glm::vec3(-0.1f,  0.1f,  0.1f)},
	Vertex{glm::vec3(-0.1f,  0.1f, -0.1f)},
	Vertex{glm::vec3(0.1f,  0.1f, -0.1f)},
	Vertex{glm::vec3(0.1f,  0.1f,  0.1f)}
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

	std::string parentDir = (fs::current_path().fs::path::parent_path()).string();
	std::string texPath = "/Resources/YoutubeOpenGL 10 - Specular Maps/";

	Texture textures[]
	{
		Texture("wood2_COLOR.jpg", "diffuse", 0, GL_RGB, GL_UNSIGNED_BYTE),

		Texture("wood2_SPEC.jpg", "specular", 1, GL_RED, GL_UNSIGNED_BYTE)
	};

	Shader shaderProgram("default.vert", "default.frag"); //creamos un objeto de la clase shader y le pasamos los archivos de shader de vertice y fragmento
	std::vector <Vertex> verts(vertices, vertices + sizeof(vertices) / sizeof(Vertex)); //creamos un vector de vertices y le pasamos los vertices)
	std::vector <GLuint> ind(indices, indices + sizeof(indices) / sizeof(GLuint)); //creamos un vector de indices y le pasamos los indices)
	std::vector <Texture> tex(textures, textures + sizeof(textures) / sizeof(Texture)); //creamos un vector de texturas y le pasamos las texturas)
	Mesh floor(verts, ind, tex); //creamos un objeto de la clase mesh y le pasamos los vertices, indices y texturas)


	Shader lightShader("light.vert", "light.frag"); //creamos un objeto de la clase shader y le pasamos los archivos de shader de vertice y fragmento para la luz
	std::vector <Vertex> lightVerts(lightVertices, lightVertices + sizeof(lightVertices) / sizeof(Vertex)); //creamos un vector de vertices y le pasamos los vertices de la luz)
	std::vector <GLuint> lightInd(lightIndices, lightIndices + sizeof(lightIndices) / sizeof(GLuint)); //creamos un vector de indices y le pasamos los indices de la luz)
	Mesh light(lightVerts, lightInd, tex); //creamos un objeto de la clase mesh y le pasamos los vertices, indices y texturas de la luz)


	glm::vec4 lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); //color de la luz

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




	glEnable(GL_DEPTH_TEST); //habilitamos el test de profundidad para que no se vea la parte trasera del triangulo

	Camera camera(width, height, glm::vec3(0.0f, 0.5f, 2.0f)); //creamos un objeto de la clase camera y le pasamos la posicion de la camara



	shaderProgram.Activate(); //activamos el shader)


	glUniform3f(glGetUniformLocation(shaderProgram.ID, "camPos"), camera.Posicion.x, camera.Posicion.y, camera.Posicion.z); //asignamos la posicion de la camara al shader





	while (!glfwWindowShouldClose(window)) //ciclo para que no se cierre el codigo
	{
		glClearColor(0.02f, 0.10f, 0.30f, 1.0f); //asignamos un color de fondo a la ventana
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  //limpiamos el buffer de color y profundidad para que no se vea la parte trasera del triangulo
		shaderProgram.Activate(); //usamos el programa de shader)

		camera.Inputs(window); //funcion para mover la camara con las teclas WASD


		camera.updateMatrix(45.0f, 0.1f, 100.0f, shaderProgram, "camMatrix");

		floor.Draw(shaderProgram, camera); //dibujamos la piramide
		lightShader.Activate(); //activamos el shader de la luz
		camera.updateMatrix(45.0f, 0.1f, 100.0f, lightShader, "camMatrix");
		light.Draw(lightShader, camera); //dibujamos la luz


		glfwSwapBuffers(window); //intercambiamos el buffer frontal con el trasero para ver el color designado
		glfwPollEvents(); //funcion para que la ventana no se cierre y pueda recibir eventos

	}



	shaderProgram.Delete(); //eliminamos el shader program
	lightShader.Delete(); //eliminamos el shader program de la luz


	glfwDestroyWindow(window);//funcion para destruir ventana
	glfwTerminate();
	return 0;
}