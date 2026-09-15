#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<stb/stb_image.h>
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
	-0.5f, -0.5f, 0.0f,     1.0f, 0.0f, 0.0f,	0.0f, 0.0f, // vertice inferior izquierdo
	-0.5f,  0.5f, 0.0f,     0.0f, 1.0f, 0.0f,	0.0f, 1.0f, // vertice superior izquierdo
	 0.5f,  0.5f, 0.0f,     0.0f, 0.0f, 1.0f,	1.0f, 1.0f, // vertice superior derecho
	 0.5f, -0.5f, 0.0f,     1.0f, 1.0f, 1.0f,	1.0f, 0.0f, // vertice inferior derecho
    };

	GLuint indices[] = //indices para dibujar el triangulo
	{
	0, 2, 1, // Upper triangle
	0, 3, 2 // Lower triangle
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

	VAO1.LinkAttrib(VBO1, 0, 3, GL_FLOAT, 8 * sizeof(float), (void*)0); //ubicamos los datos del VBO, definiendo posicion y color
	VAO1.LinkAttrib(VBO1, 1, 3, GL_FLOAT, 8 * sizeof(float), (void*)(3 * sizeof(float))); //asignamos el buffer de vertices al VAO
	VAO1.LinkAttrib(VBO1, 2, 2, GL_FLOAT, 8 * sizeof(float), (void*)(6 * sizeof(float))); //asignamos las coordenadas de textura al VAO

	VAO1.Unbind(); // 1. Desvincular el VAO primero para "cerrar" el estado
	VBO1.Unbind(); // 2. Desvincular VBO
	EBO1.Unbind(); // 3. desvincular EBO

	GLuint uniID = glGetUniformLocation(shaderProgram.ID, "scale");

	//textura

	int widthImg, heightImg, numColCh;

	stbi_set_flip_vertically_on_load(true);

	unsigned char* bytes = stbi_load(
		"minecraft-png-icon-16704.png",&widthImg,&heightImg,&numColCh,4
	);


	GLuint texture; //creamos un objeto de la clase GLuint para almacenar la textura
	glGenTextures(1, &texture); //generamos la textura
	glActiveTexture(GL_TEXTURE0); //activamos la textura)
	glBindTexture(GL_TEXTURE_2D, texture); //asignamos la textura a la variable texture)

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //filtro de minificacion)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //repetimos la textura en el eje x)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); //repetimos la textura en el eje y)

	// float borderColor[] = { 1.0f, 1.0f, 0.0f, 1.0f }; //color de borde
	// glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor); //asignamos el color de borde

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA,
		widthImg,
		heightImg,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		bytes
	);
	glGenerateMipmap(GL_TEXTURE_2D); //generamos mipmaps para la textura)

	stbi_image_free(bytes); //liberamos la memoria de la imagen
	glBindTexture(GL_TEXTURE_2D, 0); //desvinculamos la textura)

	GLuint tex0uni = glGetUniformLocation(shaderProgram.ID, "tex0"); //obtenemos la ubicacion de la textura en el shader)
	shaderProgram.Activate(); //activamos el shader)
	glUniform1i(tex0uni, 0); //asignamos la textura al shader

    while (!glfwWindowShouldClose(window)) //ciclo para que no se cierre el codigo
    {
		glClearColor(0.02f, 0.10f, 0.30f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		shaderProgram.Activate(); //usamos el programa de shader)
		glUniform1f(uniID, 0.5f);
		glBindTexture(GL_TEXTURE_2D, texture);
		VAO1.Bind(); //asignamos el buffer de vertices)
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); //dibujamos el triangulo)
		glfwSwapBuffers(window); //intercambiamos el buffer frontal con el trasero para ver el color designado
		glfwPollEvents(); //funcion para que la ventana no se cierre y pueda recibir eventos


      
    }

	VAO1.Delete(); //eliminamos el VAO
	EBO1.Delete(); //eliminamos el EBO
	VBO1.Delete(); //eliminamos el VBO
	
	glDeleteTextures(1, &texture); //eliminamos la textura
	shaderProgram.Delete(); //eliminamos el shader program


    glfwDestroyWindow(window);//funcion para destruir ventana
    glfwTerminate();
    return 0;
}