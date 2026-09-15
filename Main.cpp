#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
//incluimos librerias de OpenGL y GLFW para poder crear una ventana y renderizar gráficos en ella.

#include"shaderClass.h"
#include"VAO.h"
#include"EBO.h"





int main()
{
	//inicializamos GLFW y creamos una ventana de 800x800 píxeles con el título "Youtube"
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); //decimos que version de open gl usamos
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); //version menor
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //introducimos profile, un paquete de funciones. perfil core para usar funciones modernas.

    GLfloat vertices[] = //usamos el float de open gl para designar la ubicacion de los vertices
    {                                                                                
		-0.5f, -0.5f * float(sqrt(3)) / 3,     0.0f, 0.8f, 0.3f, 0.02f, // vertice izquierdo y su color
		 0.5f, -0.5f * float(sqrt(3)) / 3,     0.0f, 0.8f, 0.3f, 0.02f, // vertice derecho y su color
		 0.0f,  0.5f * float(sqrt(3)) * 2 / 3, 0.0f, 1.0f, 0.6f, 0.32f, // vertice superior, generamos un triangulo equilatero
		 -0.5f / 2, 0.5f * float(sqrt(3)) / 6, 0.0f, 0.9f, 0.45f, 0.17f, // vertice inferior
		0.5f / 2, 0.5f * float(sqrt(3)) / 6,   0.0f, 0.9f, 0.45f, 0.17f, // vertice inferior derecho
		0.0f, -0.5f * float(sqrt(3)) / 3,      0.0f, 0.8f, 0.3f, 0.02f, // vertice inferior izquierdo
    };

	GLuint indices[] = //indices para dibujar el triangulo
	{
		0, 3, 5,
		3, 2, 4,
		5, 4 , 1
	};

    GLFWwindow * window = glfwCreateWindow(800, 800, "Youtube", NULL, NULL); //especificaciones de la ventana (ancho, alto, título, pantalla completa )
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl; //error message 
		glfwTerminate(); //finalizamos el gfwl
        return -1;
    }
    glfwMakeContextCurrent(window); //reconoce la ventana como item 


	gladLoadGL(); //confirmamos que glad se ha cargado correctamente y podemos usar las funciones


	glViewport(0, 0, 800, 800); //limitar la ventana a 800x800

	Shader shaderProgram("default.vert", "default.frag"); //creamos un objeto de la clase shader y le pasamos los archivos de shader de vertice y fragmento

	VAO VAO1; //creamos un objeto de la clase VAO para almacenar los datos de los vertices
	VAO1.Bind(); //asignamos el buffer de vertices

	VBO VBO1(vertices, sizeof(vertices)); //creamos un objeto de la clase VBO para almacenar los datos de los vertices
	EBO EBO1(indices, sizeof(indices)); //creamos un objeto de la clase EBO para almacenar los datos de los indices

	VAO1.LinkAttrib(VBO1, 0, 3, GL_FLOAT, 6 * sizeof(float), (void*)0); //ubicamos los datos del VBO, definiendo posicion y color
	VAO1.LinkAttrib(VBO1, 1, 3, GL_FLOAT, 6 * sizeof(float), (void*)(3 * sizeof(float))); //asignamos el buffer de vertices al VAO
	
	VAO1.Unbind(); // 1. Desvincular el VAO primero para "cerrar" el estado
	VBO1.Unbind(); // 2. Desvincular VBO
	EBO1.Unbind(); // 3. desvincular EBO

	GLuint uniID = glGetUniformLocation(shaderProgram.ID, "scale");


    while (!glfwWindowShouldClose(window)) //ciclo para que no se cierre el codigo
    {
		glClearColor(0.02f, 0.10f, 0.30f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		shaderProgram.Activate(); //usamos el programa de shader)
		glUniform1f(uniID, 0.5f);
		VAO1.Bind(); //asignamos el buffer de vertices)
		glDrawElements(GL_TRIANGLES, 9, GL_UNSIGNED_INT, 0); //dibujamos el triangulo)
		glfwSwapBuffers(window); //intercambiamos el buffer frontal con el trasero para ver el color designado
		glfwPollEvents(); //funcion para que la ventana no se cierre y pueda recibir eventos


      
    }

	VAO1.Delete(); //eliminamos el VAO
	VBO1.Delete(); //eliminamos el VBO
	EBO1.Delete(); //eliminamos el EBO
	shaderProgram.Delete(); //eliminamos el shader program


    glfwDestroyWindow(window);//funcion para destruir ventana
    glfwTerminate();
    return 0;
}