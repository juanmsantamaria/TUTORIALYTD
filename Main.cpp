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
#include<cmath>

glm::vec3 Bezier(glm::vec3 p0, glm::vec3 p1,
	glm::vec3 p2, glm::vec3 p3, float t)
{
	float u = 1.0f - t;

	return u * u * u * p0
		+ 3.0f * u * u * t * p1
		+ 3.0f * u * t * t * p2
		+ t * t * t * p3;
}

//calculamos la direccion de movimiento sobre la curva
glm::vec3 DerivadaBezier(glm::vec3 p0, glm::vec3 p1,
	glm::vec3 p2, glm::vec3 p3, float t)
{
	float u = 1.0f - t;

	return 3.0f * u * u * (p1 - p0)
		+ 6.0f * u * t * (p2 - p1)
		+ 3.0f * t * t * (p3 - p2);
}

int main()
{
	//inicializamos GLFW
	glfwInit();

	//obtenemos la resolucion del monitor principal
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);

	const unsigned int width = mode->width; //ancho de la ventana
	const unsigned int height = mode->height; //alto de la ventana
	

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

	//creamos la ventana en pantalla completa
	GLFWwindow* window = glfwCreateWindow(width, height, "Youtube", monitor, NULL); 
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl; //error message 
		glfwTerminate(); //finalizamos el gfwl
		return -1;
	}
	glfwMakeContextCurrent(window); //reconoce la ventana como item 


	gladLoadGL(); //confirmamos que glad se ha cargado correctamente y podemos usar las funciones


	//ajustamos el viewport a la resolucion real de la pantalla
	int framebufferWidth, framebufferHeight;

	glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);

	glViewport(0, 0, framebufferWidth, framebufferHeight);

	std::string parentDir = (fs::current_path().fs::path::parent_path()).string();
	std::string texPath = "/Resources/YoutubeOpenGL 10 - Specular Maps/";

	Texture textures[] 
	{
		Texture("atlas.png", "diffuse", 0, GL_RGBA, GL_UNSIGNED_BYTE),

		Texture("atlas_SPEC.png", "specular", 1, GL_RGBA, GL_UNSIGNED_BYTE)
	};

	Shader shaderProgram("default.vert", "default.frag"); //creamos un objeto de la clase shader y le pasamos los archivos de shader de vertice y fragmento


	std::vector <Texture> tex(textures, textures + sizeof(textures) / sizeof(Texture)); //creamos un vector de texturas y le pasamos las texturas)

	World world; //creamos el mundo de bloques
	world.GenerateFlat(); //generamos el terreno plano
	world.PlaceHouse(World::HOUSE_X, World::SURFACE_Y + 1, World::HOUSE_Z); //construimos la casa de muestra
	world.PlaceHouse(World::HOUSE2_X, World::SURFACE_Y + 1, World::HOUSE2_Z); //segunda casa, igual a la primera, al lado de la parcela
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
		// ALDEANOS
		{ "Models/Villager.glb", 2.0f, glm::vec3(12.0f, World::SURFACE_Y + 1.0f, 15.0f) },
		{ "Models/Villager.glb", 2.0f, glm::vec3(28.0f, World::SURFACE_Y + 1.0f, 12.0f) },
		{ "Models/Villager.glb", 2.0f, glm::vec3(48.0f, World::SURFACE_Y + 1.0f, 17.0f) },

		// ABEJAS
		{ "Models/Bee.glb", 0.8f, glm::vec3(8.0f, World::SURFACE_Y + 1.0f, 8.0f) },
		{ "Models/Bee.glb", 0.8f, glm::vec3(24.0f, World::SURFACE_Y + 1.0f, 18.0f) },
		{ "Models/Bee.glb", 0.8f, glm::vec3(52.0f, World::SURFACE_Y + 1.0f, 10.0f) },

		// ZORROS
		{ "Models/Fox.glb", 0.5f, glm::vec3(15.0f, World::SURFACE_Y + 1.0f, 9.0f) },
		{ "Models/Fox.glb", 0.5f, glm::vec3(43.0f, World::SURFACE_Y + 1.0f, 12.0f) },

		// HAPPY GHAST
		{ "Models/Happy.glb", 6.0f, glm::vec3(32.0f, World::SURFACE_Y + 6.0f, 7.0f) },


		// POLLOS
		{ "Models/Poulet.glb", 0.006f, glm::vec3(20.0f, World::SURFACE_Y + 1.0f, 6.0f) },
		{ "Models/Poulet.glb", 0.006f, glm::vec3(36.0f, World::SURFACE_Y + 1.0f, 9.0f) },
		{ "Models/Poulet.glb", 0.006f, glm::vec3(53.0f, World::SURFACE_Y + 1.0f, 20.0f) },

		// GATOS
		{ "Models/Cat.glb", 0.2f, glm::vec3(8.0f, World::SURFACE_Y + 1.0f, 21.0f) },
		{ "Models/Cat.glb", 0.2f, glm::vec3(40.0f, World::SURFACE_Y + 1.0f, 5.0f) },

		// ALLAYS
		{ "Models/Allay.glb", 0.5f, glm::vec3(12.0f, World::SURFACE_Y + 4.0f, 23.0f) },
		{ "Models/Allay.glb", 0.5f, glm::vec3(50.0f, World::SURFACE_Y + 5.0f, 23.0f) },


		// EJERCITO DEL WITHER

		// CREEPERS
		{ "Models/Creeper.glb", 1.7f, glm::vec3(9.0f, World::SURFACE_Y + 1.0f, 45.0f) },
		{ "Models/Creeper.glb", 1.7f, glm::vec3(23.0f, World::SURFACE_Y + 1.0f, 47.0f) },
		{ "Models/Creeper.glb", 1.7f, glm::vec3(40.0f, World::SURFACE_Y + 1.0f, 45.0f) },
		{ "Models/Creeper.glb", 1.7f, glm::vec3(55.0f, World::SURFACE_Y + 1.0f, 47.0f) },

		// ZOMBIES
		{ "Models/Zombie.glb", 5.5f, glm::vec3(14.0f, World::SURFACE_Y + 1.0f, 52.0f) },
		{ "Models/Zombie.glb", 5.5f, glm::vec3(27.0f, World::SURFACE_Y + 1.0f, 53.0f) },
		{ "Models/Zombie.glb", 5.5f, glm::vec3(42.0f, World::SURFACE_Y + 1.0f, 51.0f) },
		{ "Models/Zombie.glb", 5.5f, glm::vec3(51.0f, World::SURFACE_Y + 1.0f, 55.0f) },

		// MAGMAS - extremos del mapa
		{ "Models/Magma.glb", 1.0f, glm::vec3(5.0f, World::SURFACE_Y + 1.0f, 38.0f) },
		{ "Models/Magma.glb", 1.0f, glm::vec3(7.0f, World::SURFACE_Y + 1.0f, 57.0f) },
		{ "Models/Magma.glb", 1.0f, glm::vec3(59.0f, World::SURFACE_Y + 1.0f, 39.0f) },
		{ "Models/Magma.glb", 1.0f, glm::vec3(58.0f, World::SURFACE_Y + 1.0f, 57.0f) },

		// WARDENS - guardianes del Wither
		{ "Models/Warden.glb", 5.0f, glm::vec3(17.0f, World::SURFACE_Y + 1.0f, 58.0f) },
		{ "Models/Warden.glb", 5.0f, glm::vec3(45.0f, World::SURFACE_Y + 1.0f, 58.0f) },

		// WITHER - jefe principal
		{ "Models/Wither.glb", 120.0f, glm::vec3(32.0f, World::SURFACE_Y + 1.0f, 16.0f) }
	};

	const int modelCount = sizeof(modelInfos) / sizeof(ModelInfo);
	Model models[modelCount]; //creamos un array de objetos de la clase model con el tamaño del numero de modelos que vamos a importar
	for (int i = 0; i < modelCount; i++) //ciclo para cargar los modelos
	{
		std::cout << "Directorio de trabajo: " << fs::current_path() << std::endl; //imprimimos el directorio de trabajo
		std::cout << "Buscando modelo: " << modelInfos[i].file << std::endl; //imprimimos el nombre del modelo que vamos a cargar

		if (!fs::exists(modelInfos[i].file)) //verificamos si el archivo existe
		{
			std::cout << "ERROR: No se encuentra el archivo GLB" << std::endl; //imprimimos un mensaje de error si no se encuentra el archivo
		}
		else
		{
			std::cout << "Archivo GLB encontrado" << std::endl; //imprimimos un mensaje si se encuentra el archivo
		}

		if (!models[i].Load(modelInfos[i].file, modelInfos[i].height)) //cargamos el modelo y verificamos si se cargo correctamente
			std::cout << "Error al cargar el modelo" << std::endl;
	}


	//generamos la trayectoria del primer zorro
	glm::vec3 curvaP0 = modelInfos[6].position;
	glm::vec3 curvaP1 = curvaP0 + glm::vec3(5.0f, 0.0f, 2.0f);
	glm::vec3 curvaP2 = curvaP0 + glm::vec3(-3.0f, 0.0f, 6.0f);
	glm::vec3 curvaP3 = curvaP0;

	std::vector<glm::vec3> puntosCurva;

	//calculamos los puntos de la curva
	for (int j = 0; j <= 100; j++)
	{
		float t = j / 100.0f;

		glm::vec3 punto = Bezier(
			curvaP0, curvaP1, curvaP2, curvaP3, t
		);

		punto.y += 0.05f;

		puntosCurva.push_back(punto);
	}

	//creamos los buffers de la curva
	GLuint curvaVAO, curvaVBO;

	glGenVertexArrays(1, &curvaVAO);
	glGenBuffers(1, &curvaVBO);

	glBindVertexArray(curvaVAO);
	glBindBuffer(GL_ARRAY_BUFFER, curvaVBO);

	glBufferData(
		GL_ARRAY_BUFFER,
		puntosCurva.size() * sizeof(glm::vec3),
		puntosCurva.data(),
		GL_STATIC_DRAW
	);

	glVertexAttribPointer(
		0, 3, GL_FLOAT, GL_FALSE,
		sizeof(glm::vec3), (void*)0
	);

	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);



	glEnable(GL_DEPTH_TEST); //habilitamos el test de profundidad para que no se vea la parte trasera del triangulo

	Camera camera(width, height, glm::vec3(16.0f, 8.0f, 40.0f)); //creamos un objeto de la clase camera y le pasamos la posicion de la camara (frente al mundo, un poco arriba del suelo)



	shaderProgram.Activate(); //activamos el shader)


	glUniform3f(glGetUniformLocation(shaderProgram.ID, "camPos"), camera.Posicion.x, camera.Posicion.y, camera.Posicion.z); //asignamos la posicion de la camara al shader


	// bloque que se coloca con el clic derecho (teclas 1 a 6 para cambiarlo)
	const char* blockNames[] = { "Aire", "Pasto", "Tierra", "Piedra", "Madera", "Hojas", "Tablas" }; //nombres de los bloques
	uint8_t selectedBlock = PLANKS;
	bool leftWasPressed = false; //para que un clic haga una sola accion
	bool rightWasPressed = false; //para que un clic haga una sola accion
	glfwSetWindowTitle(window, (std::string("Bloque: ") + blockNames[selectedBlock]).c_str()); //ponemos el nombre del bloque seleccionado en la ventana



	while (!glfwWindowShouldClose(window)) //ciclo para que no se cierre el codigo
	{
		glClearColor(0.02f, 0.10f, 0.30f, 1.0f); //asignamos un color de fondo a la ventana
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  //limpiamos el buffer de color y profundidad para que no se vea la parte trasera del triangulo
		shaderProgram.Activate(); //usamos el programa de shader)

		camera.Inputs(window); //funcion para mover la camara con las teclas WASD

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) //ESC cierra el programa
		{
			glfwSetWindowShouldClose(window, true); //cerramos la ventana
		}

		for (int k = 0; k < 6; k++) //teclas 1 a 6: elegimos el tipo de bloque
		{
			if (glfwGetKey(window, GLFW_KEY_1 + k) == GLFW_PRESS && selectedBlock != k + 1) //si se presiona la tecla y el bloque seleccionado no es el mismo
			{
				selectedBlock = (uint8_t)(k + 1);
				glfwSetWindowTitle(window, (std::string("Bloque: ") + blockNames[selectedBlock]).c_str()); //ponemos el nombre del bloque seleccionado en la ventana
			}
		}

		// construir y destruir: lanzamos un rayo desde la camara hacia donde mira
		bool leftNow = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS; //boton izquierdo del raton
		bool rightNow = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS; //boton derecho del raton
		bool rebuild = false;

		glm::ivec3 hit, prev;
		if ((leftNow && !leftWasPressed) || (rightNow && !rightWasPressed)) //solo en el momento del clic
		{
			if (world.Raycast(camera.Posicion, camera.orientacion, 6.0f, hit, prev))
			{
				if (leftNow && !leftWasPressed && hit.y > 0) //clic izquierdo: destruir (la capa de abajo no se puede romper)
				{
					world.SetBlock(hit.x, hit.y, hit.z, AIR); //ponemos aire en la celda donde se hizo el clic
					rebuild = true;
				}
				if (rightNow && !rightWasPressed) //clic derecho: colocar en la celda vacia anterior
				{
					glm::ivec3 camCell = glm::ivec3(glm::floor(camera.Posicion));
					if (prev != camCell && world.GetBlock(prev.x, prev.y, prev.z) == AIR) //no colocamos un bloque dentro de la camara
					{
						world.SetBlock(prev.x, prev.y, prev.z, selectedBlock); //ponemos el bloque seleccionado en la celda vacia anterior
						rebuild = true;
					}
				}
			}
		}
		leftWasPressed = leftNow; //para que un clic haga una sola accion
		rightWasPressed = rightNow; //para que un clic haga una sola accion

		if (rebuild) //si cambio algun bloque, regeneramos la malla del mundo
		{
			world.BuildMesh(worldVerts, worldInd); //generamos la malla del mundo (solo las caras visibles)
			worldMesh.Update(worldVerts, worldInd); //actualizamos la malla del mundo (solo las caras visibles)
		}


		camera.updateMatrix(45.0f, 0.1f, 100.0f, shaderProgram, "camMatrix"); //actualizamos la matriz de la camara en el shader

		//infeccion generada por el wither storm
		shaderProgram.Activate();

		float infectionTime = (float)glfwGetTime(); //tiempo de infeccion desde que se inicio el programa

		//la infeccion crece progresivamente
		float infectionRadius = glm::min(
			8.0f + infectionTime * 0.25f,
			30.0f
		);

		glUniform3fv(
			glGetUniformLocation(shaderProgram.ID, "witherPos"), //posicion del wither
			1,
			glm::value_ptr(modelInfos[30].position)
		);

		glUniform1f(
			glGetUniformLocation(shaderProgram.ID, "infectionRadius"), //radio de infeccion
			infectionRadius
		);

		glUniform1f(
			glGetUniformLocation(shaderProgram.ID, "infectionTime"), //tiempo de infeccion
			infectionTime
		);

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

			glm::vec3 direccion(0.0f); //direccion de movimiento del modelo (para rotarlo hacia donde se mueve)
			float rotacionY = 0.0f; //rotacion sobre el eje Y del modelo (para rotarlo hacia donde se mueve)
			float correccionY = 0.0f; //correccion de la rotacion sobre el eje Y del modelo (para corregir la orientacion del modelo)

			//corregimos la orientacion de los creepers en movimiento
			if ((i == 16 || i == 18) && std::string(modelInfos[i].file) == "Models/Creeper.glb") //los creepers 16 y 18 se mueven sobre curvas de Bezier
			{
				correccionY = glm::radians(180.0f); //corregimos la orientacion de los creepers en movimiento
			}

			//corregimos la orientacion de los zombies estaticos
			if ((i == 21 || i == 23) && std::string(modelInfos[i].file) == "Models/Zombie.glb") //los zombies 21 y 23 estan estaticos
			{
				correccionY = glm::radians(180.0f); //corregimos la orientacion de los zombies estaticos
			}

			//rotacion suave de los magmas sobre el eje Y
			if (std::string(modelInfos[i].file) == "Models/Magma.glb") //los magmas giran sobre el eje Y
			{
				float tiempo = (float)glfwGetTime() + i * 2.0f; //tiempo de rotacion
				rotacionY = glm::radians(30.0f) * std::sin(tiempo * 0.8f); //rotacion sobre el eje Y
			}

			//movimiento de dos zombies mediante curvas de Bezier
			if ((i == 20 || i == 22) && std::string(modelInfos[i].file) == "Models/Zombie.glb") //los zombies 20 y 22 se mueven sobre curvas de Bezier
			{
				float tiempo = (float)glfwGetTime() + i * 3.0f; //tiempo de movimiento
				float t = std::fmod(tiempo / 20.0f, 1.0f); //parametro de la curva de Bezier (0 a 1)

				glm::vec3 p0 = modelInfos[i].position; //punto de inicio de la curva
				glm::vec3 p1 = p0 + glm::vec3(15.0f, 0.0f, -12.0f); //punto de control 1 de la curva
				glm::vec3 p2 = p0 + glm::vec3(-15.0f, 0.0f, -12.0f); //punto de control 2 de la curva
				glm::vec3 p3 = p0; //punto final de la curva (igual al punto de inicio para cerrar el ciclo)

				villagerPos = Bezier(p0, p1, p2, p3, t); //calculamos la posicion del zombie sobre la curva de Bezier
				direccion = DerivadaBezier(p0, p1, p2, p3, t); //calculamos la direccion de movimiento del zombie sobre la curva de Bezier
			}

			//movimiento de dos creepers con velocidad variable

			if ((i == 16 || i == 18) && std::string(modelInfos[i].file) == "Models/Creeper.glb") //	los creepers 16 y 18 se mueven sobre curvas de Bezier
			{
				float tiempo = (float)glfwGetTime(); //tiempo de movimiento
				float t = std::fmod(tiempo / (i == 16 ? 12.0f : 16.0f), 2.0f); //parametro de la curva de Bezier (0 a 2, para que el creeper regrese al punto de inicio)

				bool regreso = t > 1.0f; //si t es mayor a 1, el creeper esta regresando al punto de inicio

				if (regreso) //	si el creeper esta regresando al punto de inicio
					t = 2.0f - t;

				//aceleracion y desaceleracion
				t = t * t * (3.0f - 2.0f * t);

				glm::vec3 p0 = modelInfos[i].position; //punto de inicio de la curva
				glm::vec3 p1 = p0 + glm::vec3(4.0f, 0.0f, -8.0f); //punto de control 1 de la curva
				glm::vec3 p2 = p0 + glm::vec3(12.0f, 0.0f, -10.0f); //punto de control 2 de la curva
				glm::vec3 p3 = p0 + glm::vec3(15.0f, 0.0f, -5.0f); //	punto final de la curva (distinto al punto de inicio para que el creeper se mueva hacia adelante)

				villagerPos = Bezier(p0, p1, p2, p3, t); //calculamos la posicion del creeper sobre la curva de Bezier
				direccion = DerivadaBezier(p0, p1, p2, p3, t); //calculamos la direccion de movimiento del creeper sobre la curva de Bezier

				//invertimos la direccion cuando regresa
				if (regreso) //si el creeper esta regresando al punto de inicio
					direccion = -direccion; //invertimos la direccion de movimiento del creeper
			}

			//vuelo tridimensional de la primera abeja
			//vuelo tridimensional del Happy Ghast
			if (std::string(modelInfos[i].file) == "Models/Happy.glb") //el Happy Ghast se mueve sobre una curva de Bezier
			{
				float tiempo = (float)glfwGetTime(); //tiempo de movimiento
				float t = std::fmod(tiempo / 35.0f, 1.0f); //parametro de la curva de Bezier (0 a 1)

				glm::vec3 p0 = modelInfos[i].position; //punto de inicio de la curva
				glm::vec3 p1 = p0 + glm::vec3(30.0f, 18.0f, 20.0f); //punto de control 1 de la curva
				glm::vec3 p2 = p0 + glm::vec3(-30.0f, 8.0f, 30.0f); //punto de control 2 de la curva
				glm::vec3 p3 = p0; //	punto final de la curva (igual al punto de inicio para cerrar el ciclo)
				 
				villagerPos = Bezier(p0, p1, p2, p3, t); //calculamos la posicion del Happy Ghast sobre la curva de Bezier
			}

			//movimiento de los zorros
			if (std::string(modelInfos[i].file) == "Models/Fox.glb")
			{
				float tiempo = (float)glfwGetTime() + i * 2.0f;
				float t = std::fmod(tiempo / 8.0f, 1.0f);

				glm::vec3 p0 = modelInfos[i].position;
				glm::vec3 p1 = p0 + glm::vec3(5.0f, 0.0f, 2.0f);
				glm::vec3 p2 = p0 + glm::vec3(-3.0f, 0.0f, 6.0f);
				glm::vec3 p3 = p0;

				villagerPos = Bezier(p0, p1, p2, p3, t);
				direccion = DerivadaBezier(p0, p1, p2, p3, t);
			}

			//vuelo de los allays
			if (std::string(modelInfos[i].file) == "Models/Allay.glb")
			{
				float tiempo = (float)glfwGetTime() + i * 2.0f;
				float t = std::fmod(tiempo / 12.0f, 1.0f);

				glm::vec3 p0 = modelInfos[i].position;
				glm::vec3 p1 = p0 + glm::vec3(6.0f, 8.0f, 4.0f);
				glm::vec3 p2 = p0 + glm::vec3(-6.0f, 2.0f, 8.0f);
				glm::vec3 p3 = p0;

				villagerPos = Bezier(p0, p1, p2, p3, t);
				direccion = DerivadaBezier(p0, p1, p2, p3, t);
			}

			//orientamos el warden izquierdo hacia el wither
			if (i == 28 && std::string(modelInfos[i].file) == "Models/Warden.glb")
				direccion = modelInfos[30].position - villagerPos;

			//calculamos el angulo de orientacion sobre el eje Y
			if (std::abs(direccion.x) > 0.001f ||
				std::abs(direccion.z) > 0.001f)
			{
				rotacionY = std::atan2(direccion.x, direccion.z);
			}

			//trasladamos y rotamos el modelo
			glm::mat4 villagerMatrix = glm::translate(glm::mat4(1.0f), villagerPos);

			villagerMatrix = glm::rotate(
				villagerMatrix,
				rotacionY + correccionY,
				glm::vec3(0.0f, 1.0f, 0.0f)
			);

			models[i].Draw(modelShader, villagerMatrix);
		}

		//dibujamos la curva de Bezier
		lightShader.Activate();

		camera.updateMatrix(
			45.0f, 0.1f, 100.0f,
			lightShader, "camMatrix"
		);

		//matriz identidad para la curva
		glm::mat4 curvaModel = glm::mat4(1.0f);

		glUniformMatrix4fv(
			glGetUniformLocation(lightShader.ID, "model"),
			1, GL_FALSE, glm::value_ptr(curvaModel)
		);

		//color amarillo de la curva
		glUniform4f(
			glGetUniformLocation(lightShader.ID, "lightColor"),
			1.0f, 1.0f, 0.0f, 1.0f
		);

		//dibujamos los puntos conectados mediante lineas
		glBindVertexArray(curvaVAO);

		glDrawArrays(
			GL_LINE_STRIP,
			0,
			(GLsizei)puntosCurva.size()
		);

		glBindVertexArray(0);

		//restauramos las propiedades de la luz
		glUniformMatrix4fv(
			glGetUniformLocation(lightShader.ID, "model"),
			1, GL_FALSE, glm::value_ptr(lightModel)
		);

		glUniform4f(
			glGetUniformLocation(lightShader.ID, "lightColor"),
			lightColor.x, lightColor.y, lightColor.z, 1.0f
		);

		// mira en el centro de la pantalla (dos rectangulos blancos pequeños)
		glEnable(GL_SCISSOR_TEST);  //habilitamos el test de recorte para dibujar solo en una parte de la pantalla
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f); //asignamos un color blanco para la mira
		glScissor(width / 2 - 10, height / 2 - 1, 20, 2); //definimos un rectangulo de 20x2 pixeles en el centro de la pantalla
		glClear(GL_COLOR_BUFFER_BIT); //limpiamos el buffer de color solo en el rectangulo definido
		glScissor(width / 2 - 1, height / 2 - 10, 2, 20); //definimos un rectangulo de 2x20 pixeles en el centro de la pantalla
		glClear(GL_COLOR_BUFFER_BIT); //limpiamos el buffer de color solo en el rectangulo definido
		glDisable(GL_SCISSOR_TEST); //deshabilitamos el test de recorte para dibujar en toda la pantalla


		glfwSwapBuffers(window); //intercambiamos el buffer frontal con el trasero para ver el color designado
		glfwPollEvents(); //funcion para que la ventana no se cierre y pueda recibir eventos

	}

	//liberamos los buffers de la curva
	glDeleteVertexArrays(1, &curvaVAO);
	glDeleteBuffers(1, &curvaVBO);

	shaderProgram.Delete(); //eliminamos el shader program
	lightShader.Delete(); //eliminamos el shader program de la luz
	modelShader.Delete(); //eliminamos el shader program de los modelos
	for (int i = 0; i < modelCount; i++)
		models[i].Delete(); //liberamos los modelos de la GPU


	glfwDestroyWindow(window);//funcion para destruir ventana
	glfwTerminate();
	return 0;
}