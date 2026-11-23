#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<filesystem>
namespace fs = std::filesystem;
#include"Mesh.h"
#include"World.h"
#include"Model.h"




int main()
{
	const unsigned int width = 800;
	const unsigned int height = 800;
	//inicializamos GLFW y creamos una ventana de 800x800 píxeles con el título "Youtube"
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); //decimos que version de open gl usamos
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); //version menor
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //introducimos profile, un paquete de funciones. perfil core para usar funciones modernas.

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
	
	

	std::vector <Texture> tex(textures, textures + sizeof(textures) / sizeof(Texture)); //creamos un vector de texturas y le pasamos las texturas)

	World world; //creamos el mundo de bloques
	world.GenerateFlat(); //generamos el terreno plano
	world.PlaceHouse(World::HOUSE_X, World::SURFACE_Y + 1, World::HOUSE_Z); //construimos la casa de muestra
	world.MarkPlot(World::PLOT_X, World::PLOT_Z); //marcamos el terreno donde se construira la copia
	world.PlaceTrees(18, 1234); //plantamos 6 arboles (misma semilla = mismo bosque)
	std::vector <Vertex> worldVerts; //vector de vertices del mundo
	std::vector <GLuint> worldInd; //vector de indices del mundo
	world.BuildMesh(worldVerts, worldInd); //generamos la malla del mundo (solo las caras visibles)
	Mesh worldMesh(worldVerts, worldInd, tex); //creamos un objeto de la clase mesh con la malla del mundo)


	Shader lightShader("light.vert", "light.frag"); //creamos un objeto de la clase shader y le pasamos los archivos de shader de vertice y fragmento para la luz
	std::vector <Vertex> lightVerts(lightVertices, lightVertices + sizeof(lightVertices) / sizeof(Vertex)); //creamos un vector de vertices y le pasamos los vertices de la luz)
	std::vector <GLuint> lightInd(lightIndices, lightIndices + sizeof(lightIndices) / sizeof(GLuint)); //creamos un vector de indices y le pasamos los indices de la luz)
	Mesh light(lightVerts, lightInd, tex); //creamos un objeto de la clase mesh y le pasamos los vertices, indices y texturas de la luz)


	glm::vec4 lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); //color de la luz

	glm::vec3 lightPos = glm::vec3(32.0f, 25.0f, 32.0f); //posicion de la luz (encima del centro del mundo)
	glm::mat4 lightModel = glm::mat4(1.0f); //matriz de modelo de la luz
	lightModel = glm::translate(lightModel, lightPos); //trasladamos la luz

	glm::vec3 pyramidPos = glm::vec3(0.0f, 0.0f, 0.0f); //posicion del mundo
	glm::mat4 pyramidModel = glm::mat4(1.0f); //matriz de modelo del mundo
	pyramidModel = glm::translate(pyramidModel, pyramidPos); //trasladamos el mundo

	lightShader.Activate(); //activamos el shader de la luz
	glUniformMatrix4fv(glGetUniformLocation(lightShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(lightModel));
	glUniform4f(glGetUniformLocation(lightShader.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, 1.0f); //asignamos el color de la luz al shader)
	shaderProgram.Activate(); //activamos el shader del mundo
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "model"), 1, GL_FALSE, glm::value_ptr(pyramidModel));
	glUniform4f(glGetUniformLocation(shaderProgram.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, 1.0f); //asignamos el color de la luz al shader)
	glUniform3f(glGetUniformLocation(shaderProgram.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z); //asignamos la posicion de la luz al shader)

	Shader modelShader("model.vert", "model.frag"); //shader de los modelos importados (con huesos)
	modelShader.Activate(); //activamos el shader de los modelos
	glUniform4f(glGetUniformLocation(modelShader.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, 1.0f); //color de la luz
	glUniform3f(glGetUniformLocation(modelShader.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z); //posicion de la luz

	// modelos importados: carpeta con el .glb, alto en bloques y posicion a nivel de piso
	struct ModelInfo { const char* file; float height; glm::vec3 position; };

	ModelInfo modelInfos[] =
	{
		{ "Models/Villager.glb", 2.0f, glm::vec3(12.0f, World::SURFACE_Y + 1.0f, 10.0f) },
		{ "Models/Bee.glb", 0.8f, glm::vec3(15.0f, World::SURFACE_Y + 1.0f, 12.0f) },
		{ "Models/Creeper.glb", 1.7f, glm::vec3(20.0f, World::SURFACE_Y + 1.0f, 30.0f) },
		{ "Models/Zombie.glb", 5.5f, glm::vec3(22.0f, World::SURFACE_Y + 1.0f, 30.0f) },
		{ "Models/Fox.glb", 1.0f, glm::vec3(16.0f, World::SURFACE_Y + 1.0f, 13.0f) },
		{ "Models/Happy.glb", 1.0f, glm::vec3(16.0f, World::SURFACE_Y + 3.0f, 8.0f) }
	};

	const int modelCount = sizeof(modelInfos) / sizeof(ModelInfo);
	Model models[modelCount];
	for (int i = 0; i < modelCount; i++)
	{
		std::cout << "Directorio de trabajo: " << fs::current_path() << std::endl;
		std::cout << "Buscando modelo: " << modelInfos[i].file << std::endl;

		if (!fs::exists(modelInfos[i].file))
		{
			std::cout << "ERROR: No se encuentra el archivo GLB" << std::endl;
		}
		else
		{
			std::cout << "Archivo GLB encontrado" << std::endl;
		}

		if (!models[i].Load(modelInfos[i].file, modelInfos[i].height))
			std::cout << "Error al cargar el modelo" << std::endl;
	}




	glEnable(GL_DEPTH_TEST); //habilitamos el test de profundidad para que no se vea la parte trasera del triangulo

	Camera camera(width, height, glm::vec3(16.0f, 8.0f, 40.0f)); //creamos un objeto de la clase camera y le pasamos la posicion de la camara (frente al mundo, un poco arriba del suelo)



	shaderProgram.Activate(); //activamos el shader)


	glUniform3f(glGetUniformLocation(shaderProgram.ID, "camPos"), camera.Posicion.x, camera.Posicion.y, camera.Posicion.z); //asignamos la posicion de la camara al shader


	// bloque que se coloca con el clic derecho (teclas 1 a 6 para cambiarlo)
	const char* blockNames[] = { "Aire", "Pasto", "Tierra", "Piedra", "Madera", "Hojas", "Tablas" };
	uint8_t selectedBlock = PLANKS;
	bool leftWasPressed = false; //para que un clic haga una sola accion
	bool rightWasPressed = false;
	glfwSetWindowTitle(window, (std::string("Bloque: ") + blockNames[selectedBlock]).c_str());



	while (!glfwWindowShouldClose(window)) //ciclo para que no se cierre el codigo
	{
		glClearColor(0.02f, 0.10f, 0.30f, 1.0f); //asignamos un color de fondo a la ventana
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  //limpiamos el buffer de color y profundidad para que no se vea la parte trasera del triangulo
		shaderProgram.Activate(); //usamos el programa de shader)

		camera.Inputs(window); //funcion para mover la camara con las teclas WASD

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) //ESC cierra el programa
		{
			glfwSetWindowShouldClose(window, true);
		}

		for (int k = 0; k < 6; k++) //teclas 1 a 6: elegimos el tipo de bloque
		{
			if (glfwGetKey(window, GLFW_KEY_1 + k) == GLFW_PRESS && selectedBlock != k + 1)
			{
				selectedBlock = (uint8_t)(k + 1);
				glfwSetWindowTitle(window, (std::string("Bloque: ") + blockNames[selectedBlock]).c_str());
			}
		}

		// construir y destruir: lanzamos un rayo desde la camara hacia donde mira
		bool leftNow = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
		bool rightNow = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
		bool rebuild = false;

		glm::ivec3 hit, prev;
		if ((leftNow && !leftWasPressed) || (rightNow && !rightWasPressed)) //solo en el momento del clic
		{
			if (world.Raycast(camera.Posicion, camera.orientacion, 6.0f, hit, prev))
			{
				if (leftNow && !leftWasPressed && hit.y > 0) //clic izquierdo: destruir (la capa de abajo no se puede romper)
				{
					world.SetBlock(hit.x, hit.y, hit.z, AIR);
					rebuild = true;
				}
				if (rightNow && !rightWasPressed) //clic derecho: colocar en la celda vacia anterior
				{
					glm::ivec3 camCell = glm::ivec3(glm::floor(camera.Posicion));
					if (prev != camCell && world.GetBlock(prev.x, prev.y, prev.z) == AIR) //no colocamos un bloque dentro de la camara
					{
						world.SetBlock(prev.x, prev.y, prev.z, selectedBlock);
						rebuild = true;
					}
				}
			}
		}
		leftWasPressed = leftNow;
		rightWasPressed = rightNow;

		if (rebuild) //si cambio algun bloque, regeneramos la malla del mundo
		{
			world.BuildMesh(worldVerts, worldInd);
			worldMesh.Update(worldVerts, worldInd);
		}


		camera.updateMatrix(45.0f, 0.1f, 100.0f, shaderProgram, "camMatrix");

		worldMesh.Draw(shaderProgram, camera); //dibujamos el mundo
		lightShader.Activate(); //activamos el shader de la luz
		camera.updateMatrix(45.0f, 0.1f, 100.0f, lightShader, "camMatrix");
		glDisable(GL_CULL_FACE); //mostramos ambas caras del modelo para comprobar su geometria
		light.Draw(lightShader, camera); //dibujamos la luz

		modelShader.Activate(); //activamos el shader de los modelos
		camera.updateMatrix(45.0f, 0.1f, 100.0f, modelShader, "camMatrix");
		glDisable(GL_CULL_FACE); //mostramos las dos caras del modelo

		for (int i = 0; i < modelCount; i++)
		{
			if (models[i].AnimationCount() > 0)
				models[i].Update((float)glfwGetTime(), 0); //reproducimos la primera animacion disponible
			else
				models[i].Update(0.0f, -1); //mostramos la pose de reposo si no tiene animaciones

			glm::vec3 villagerPos = modelInfos[i].position; //posicion del modelo definida en modelInfos

			glm::mat4 villagerMatrix = glm::translate(glm::mat4(1.0f), villagerPos);
			models[i].Draw(modelShader, villagerMatrix); //dibujamos el aldeano en su posicion
		}

		// mira en el centro de la pantalla (dos rectangulos blancos pequeños)
		glEnable(GL_SCISSOR_TEST);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glScissor(width / 2 - 10, height / 2 - 1, 20, 2);
		glClear(GL_COLOR_BUFFER_BIT);
		glScissor(width / 2 - 1, height / 2 - 10, 2, 20);
		glClear(GL_COLOR_BUFFER_BIT);
		glDisable(GL_SCISSOR_TEST);


		glfwSwapBuffers(window); //intercambiamos el buffer frontal con el trasero para ver el color designado
		glfwPollEvents(); //funcion para que la ventana no se cierre y pueda recibir eventos

	}



	shaderProgram.Delete(); //eliminamos el shader program
	lightShader.Delete(); //eliminamos el shader program de la luz
	modelShader.Delete(); //eliminamos el shader program de los modelos
	for (int i = 0; i < modelCount; i++)
		models[i].Delete(); //liberamos los modelos de la GPU


	glfwDestroyWindow(window);//funcion para destruir ventana
	glfwTerminate();
	return 0;
}
