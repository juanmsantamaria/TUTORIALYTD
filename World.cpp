#include"World.h"
#include<cstring>
#include<cfloat>
#include<cstdlib>

// una cara del cubo: direccion del vecino, normal y las 4 esquinas (en sentido antihorario visto desde afuera)
struct Face
{
	int dx, dy, dz;
	glm::vec3 normal; //normal de la cara
	glm::vec3 corners[4];
};

static const Face FACES[6] =
{
	// arriba (+Y)
	{  0,  1,  0, glm::vec3(0,  1,  0), { glm::vec3(0, 1, 1), glm::vec3(1, 1, 1), glm::vec3(1, 1, 0), glm::vec3(0, 1, 0) } },
	// abajo (-Y)
	{  0, -1,  0, glm::vec3(0, -1,  0), { glm::vec3(0, 0, 0), glm::vec3(1, 0, 0), glm::vec3(1, 0, 1), glm::vec3(0, 0, 1) } },
	// frente (+Z)
	{  0,  0,  1, glm::vec3(0,  0,  1), { glm::vec3(0, 0, 1), glm::vec3(1, 0, 1), glm::vec3(1, 1, 1), glm::vec3(0, 1, 1) } },
	// atras (-Z)
	{  0,  0, -1, glm::vec3(0,  0, -1), { glm::vec3(1, 0, 0), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), glm::vec3(1, 1, 0) } },
	// derecha (+X)
	{  1,  0,  0, glm::vec3(1,  0,  0), { glm::vec3(1, 0, 1), glm::vec3(1, 0, 0), glm::vec3(1, 1, 0), glm::vec3(1, 1, 1) } },
	// izquierda (-X)
	{ -1,  0,  0, glm::vec3(-1,  0,  0), { glm::vec3(0, 0, 0), glm::vec3(0, 0, 1), glm::vec3(0, 1, 1), glm::vec3(0, 1, 0) } }
};

// coordenadas de textura de las 4 esquinas de cada cara
static const glm::vec2 UVS[4] =
{
	glm::vec2(0.0f, 0.0f), // esquina inferior izquierda
	glm::vec2(1.0f, 0.0f), // esquina inferior derecha
	glm::vec2(1.0f, 1.0f), // esquina superior derecha
	glm::vec2(0.0f, 1.0f) // esquina superior izquierda
};

// color con el que se tiñe la textura de cada tipo de bloque (provisional, hasta tener un atlas)
//seleccionamos la textura dependiendo del bloque y de la cara
static int BlockTexture(uint8_t type, int face)
{
	switch (type) //seleccionamos la textura dependiendo del bloque y de la cara
	{
	case STONE: //textura de piedra
		return 0;

	case DIRT: //textura de tierra
		return 1;

	case GRASS: //textura de pasto
		if (face == 0) return 2; //cara de arriba: pasto
		if (face == 1) return 1; //cara de abajo: tierra
		return 3;

	case LEAVES: //textura de hojas
		return 4;

	case WOOD: //textura de madera
		if (face == 0 || face == 1) return 6; //caras de arriba y abajo: corteza
		return 5;

	case PLANKS:
		return 7;

	default:
		return 0;
	}
}

//calculamos las coordenadas UV dentro del atlas
static glm::vec2 AtlasUV(int texture, glm::vec2 uv)
{
	float column = (float)(texture % 4); //columna de la textura en el atlas
	float row = (float)(texture / 4); //fila de la textura en el atlas

	float u = (column * 1024.0f + 0.5f + uv.x * 1023.0f) / 4096.0f; //calculamos las coordenadas UV dentro del atlas de texturas
	float v = ((1.0f - row) * 1024.0f + 0.5f + uv.y * 1023.0f) / 2048.0f; //calculamos las coordenadas UV dentro del atlas de texturas

	return glm::vec2(u, v); //calculamos las coordenadas UV dentro del atlas de texturas
}

World::World()  // constructor de la clase World
{
	std::memset(blocks, AIR, sizeof(blocks)); //inicializamos todos los bloques a AIR
}

void World::GenerateFlat() // generamos un terreno plano: piedra, dos capas de tierra y pasto arriba
{
	for (int x = 0; x < SIZE_X; x++) // recorremos todas las filas del mundo
	{
		for (int z = 0; z < SIZE_Z; z++) // recorremos todas las columnas del mundo
		{
			blocks[x][0][z] = STONE; // capa de piedra
			blocks[x][1][z] = DIRT; // capa de tierra
			blocks[x][2][z] = DIRT; // capa de tierra
			blocks[x][3][z] = GRASS; // capa de pasto
		}
	}
}

uint8_t World::GetBlock(int x, int y, int z) const
{
	if (x < 0 || x >= SIZE_X || y < 0 || y >= SIZE_Y || z < 0 || z >= SIZE_Z) // si la posicion esta fuera del mundo, devolvemos AIR
		return AIR;
	return blocks[x][y][z]; // devolvemos el tipo de bloque en la posicion (x, y, z)
}

void World::SetBlock(int x, int y, int z, uint8_t type) // cambiamos un bloque (no hace nada si la posicion esta fuera del mundo)
{
	if (x < 0 || x >= SIZE_X || y < 0 || y >= SIZE_Y || z < 0 || z >= SIZE_Z) // no hacemos nada si la posicion esta fuera del mundo
		return;
	blocks[x][y][z] = type; // cambiamos el tipo de bloque en la posicion (x, y, z)
}

bool World::IsOpaque(int x, int y, int z) const
{
	if (y < 0) return true; // debajo del mundo: no dibujamos la cara de abajo
	return GetBlock(x, y, z) != AIR;
}

void World::BuildMesh(std::vector<Vertex>& vertices, std::vector<GLuint>& indices) const //generamos la malla del mundo, solo con las caras visibles
{
	vertices.clear(); //limpiamos el vector de vertices
	indices.clear();

	for (int x = 0; x < SIZE_X; x++) // recorremos todas las columnas del mundo
	{
		for (int y = 0; y < SIZE_Y; y++) // recorremos todas las capas del mundo
		{
			for (int z = 0; z < SIZE_Z; z++) // recorremos todos los bloques del mundo
			{
				uint8_t type = blocks[x][y][z]; //	tipo de bloque en la posicion (x, y, z)
				if (type == AIR) continue;

				glm::vec3 color(1.0f, 1.0f, 1.0f); //color con el que se tiñe la textura de cada tipo de bloque (provisional, hasta tener un atlas)

				//conservamos el color original de las tablas
				if (type == PLANKS)
					color = glm::vec3(1.00f, 0.85f, 0.55f);

				for (int f = 0; f < 6; f++) // seis caras del cubo
				{
					const Face& face = FACES[f]; // cara del cubo

					// si el vecino tapa esta cara, no la dibujamos
					if (IsOpaque(x + face.dx, y + face.dy, z + face.dz)) continue; 

					GLuint base = (GLuint)vertices.size(); // indice del primer vertice de la cara

					for (int c = 0; c < 4; c++) // cuatro esquinas de la cara
					{
						glm::vec3 pos = glm::vec3((float)x, (float)y, (float)z) + face.corners[c];
						//seleccionamos la textura correspondiente a la cara
						int texture = BlockTexture(type, f); //calculamos la textura dependiendo del bloque y de la cara
						glm::vec2 uv = AtlasUV(texture, UVS[c]); //calculamos las coordenadas UV dentro del atlas

						vertices.push_back(Vertex{ pos, face.normal, color, uv });
					}

					indices.push_back(base); // dos triangulos por cara
					indices.push_back(base + 1); // dos triangulos por cara
					indices.push_back(base + 2); // dos triangulos por cara
					indices.push_back(base); // dos triangulos por cara
					indices.push_back(base + 2); // dos triangulos por cara
					indices.push_back(base + 3); // dos triangulos por cara
				}
			}
		}
	}
}

// recorre la cuadricula celda por celda a lo largo del rayo (algoritmo DDA)
bool World::Raycast(glm::vec3 origin, glm::vec3 direction, float maxDistance, glm::ivec3& hit, glm::ivec3& previous) const
{
	glm::vec3 dir = glm::normalize(direction);

	glm::ivec3 cell = glm::ivec3(glm::floor(origin)); // celda donde empieza el rayo
	glm::ivec3 prevCell = cell;
	glm::ivec3 step(0);
	glm::vec3 tMax(FLT_MAX);   // distancia hasta cruzar el siguiente borde de celda en cada eje
	glm::vec3 tDelta(FLT_MAX); // distancia necesaria para cruzar una celda completa en cada eje

	for (int i = 0; i < 3; i++) // calculamos los pasos y las distancias para cada eje
	{
		if (dir[i] > 0.0f) // si el rayo va en direccion positiva
		{
			step[i] = 1; // paso positivo en el eje i
			tMax[i] = ((float)(cell[i] + 1) - origin[i]) / dir[i]; // distancia hasta el siguiente borde de celda en el eje i
			tDelta[i] = 1.0f / dir[i]; // distancia necesaria para cruzar una celda completa en el eje i
		}
		else if (dir[i] < 0.0f) // si el rayo va en direccion negativa
		{
			step[i] = -1; // paso negativo en el eje i
			tMax[i] = (origin[i] - (float)cell[i]) / -dir[i]; // distancia hasta el siguiente borde de celda en el eje i
			tDelta[i] = 1.0f / -dir[i]; // distancia necesaria para cruzar una celda completa en el eje i
		}
	}

	float t = 0.0f; // distancia recorrida por el rayo
	while (t <= maxDistance) // mientras no hayamos superado la distancia maxima
	{
		if (GetBlock(cell.x, cell.y, cell.z) != AIR) //	si la celda actual contiene un bloque, devolvemos la celda y la anterior
		{
			hit = cell; // celda donde golpea el rayo
			previous = prevCell; // celda anterior al golpe
			return true; // golpea un bloque
		}

		prevCell = cell;

		// avanzamos por el eje cuyo borde esta mas cerca
		int axis;
		if (tMax.x < tMax.y)
			axis = (tMax.x < tMax.z) ? 0 : 2; // eje X o Z
		else
			axis = (tMax.y < tMax.z) ? 1 : 2; // eje Y o Z

		t = tMax[axis]; // actualizamos la distancia recorrida por el rayo
		cell[axis] += step[axis]; //	avanzamos a la siguiente celda en el eje correspondiente
		tMax[axis] += tDelta[axis]; // actualizamos la distancia hasta el siguiente borde de celda en el eje correspondiente
	}

	return false;
}

// true si (x, z) cae dentro de un area rectangular (mas un margen)
static bool InsideArea(int x, int z, int ox, int oz, int w, int d, int margin)
{
	return x >= ox - margin && x < ox + w + margin && 
		z >= oz - margin && z < oz + d + margin; // true si (x, z) cae dentro de un area rectangular (mas un margen)
}

void World::PlaceHouse(int ox, int oy, int oz)
{
	const int W = HOUSE_W;
	const int D = HOUSE_D;

	// piso de tablas
	for (int x = 0; x < W; x++)
		for (int z = 0; z < D; z++)
			SetBlock(ox + x, oy, oz + z, PLANKS);

	// paredes de 4 bloques de alto: esquinas de madera y el resto de tablas
	for (int y = 1; y <= 4; y++)
	{
		for (int x = 0; x < W; x++)
		{
			for (int z = 0; z < D; z++)
			{
				bool edge = (x == 0 || x == W - 1 || z == 0 || z == D - 1);
				if (!edge) continue;
				bool corner = (x == 0 || x == W - 1) && (z == 0 || z == D - 1);
				SetBlock(ox + x, oy + y, oz + z, corner ? WOOD : PLANKS);
			}
		}
	}

	// puerta en el centro de la pared del frente (+Z), de 2 bloques de alto
	SetBlock(ox + W / 2, oy + 1, oz + D - 1, AIR);
	SetBlock(ox + W / 2, oy + 2, oz + D - 1, AIR);

	// ventanas
	SetBlock(ox + 1, oy + 2, oz + D - 1, AIR);     // frente, izquierda
	SetBlock(ox + W - 2, oy + 2, oz + D - 1, AIR); // frente, derecha
	SetBlock(ox, oy + 2, oz + D / 2, AIR);         // lado izquierdo
	SetBlock(ox + W - 1, oy + 2, oz + D / 2, AIR); // lado derecho
	SetBlock(ox + W / 2, oy + 2, oz, AIR);         // pared de atras

	// techo escalonado de piedra: 7x7, 5x5, 3x3 y 1x1
	for (int layer = 0; layer < 4; layer++)
		for (int x = layer; x < W - layer; x++)
			for (int z = layer; z < D - layer; z++)
				SetBlock(ox + x, oy + 5 + layer, oz + z, STONE);
}

void World::MarkPlot(int ox, int oz)
{
	// marco de piedra en el suelo, justo afuera de la parcela
	for (int x = -1; x <= PLOT_W; x++)
		for (int z = -1; z <= PLOT_D; z++)
			if (x == -1 || x == PLOT_W || z == -1 || z == PLOT_D)
				SetBlock(ox + x, SURFACE_Y, oz + z, STONE);
}

void World::PlaceTree(int x, int z)
{

	int base = SURFACE_Y + 1;     // primer bloque sobre el pasto
	int top = base + 3;           // tronco de 4 bloques

	// hojas: dos capas de 5x5 (sin esquinas), una de 3x3 y una cruz arriba
	for (int dy = -1; dy <= 2; dy++)
	{
		int r = (dy <= 0) ? 2 : 1;
		for (int dx = -r; dx <= r; dx++) // capa de hojas de 5x5 o 3x3
		{
			for (int dz = -r; dz <= r; dz++) // capa de hojas de 5x5 o 3x3
			{
				bool corner = (std::abs(dx) == r && std::abs(dz) == r); // true si es una esquina
				if (corner && (dy <= 0 || dy == 2)) continue;
				if (GetBlock(x + dx, top + dy, z + dz) == AIR)
					SetBlock(x + dx, top + dy, z + dz, LEAVES);
			}
		}
	}

	// tronco
	for (int y = base; y <= top; y++) // tronco de 4 bloques
		SetBlock(x, y, z, WOOD);
}

void World::PlaceTrees(int count, unsigned int seed) // coloca arboles aleatoriamente en el mundo, evitando la casa y la parcela
{
	unsigned int state = seed; // generador de numeros pseudoaleatorios sencillo (misma semilla = mismo resultado)
	std::vector<glm::ivec2> placed; // lista de posiciones donde ya se han colocado arboles

	for (int attempt = 0; attempt < 500 && (int)placed.size() < count; attempt++)
	{
		// generador de numeros pseudoaleatorios sencillo (misma semilla = mismo resultado)
		state = state * 1664525u + 1013904223u;
		int x = 3 + (int)((state >> 16) % (unsigned int)(SIZE_X - 6));
		state = state * 1664525u + 1013904223u;
		int z = 3 + (int)((state >> 16) % (unsigned int)(SIZE_Z - 6));

		// no plantamos sobre la casa ni sobre la parcela
		if (InsideArea(x, z, HOUSE_X, HOUSE_Z, HOUSE_W, HOUSE_D, 3)) continue;
		if (InsideArea(x, z, PLOT_X, PLOT_Z, PLOT_W, PLOT_D, 3)) continue;
		if (InsideArea(x, z, HOUSE2_X, HOUSE2_Z, HOUSE_W, HOUSE_D, 3)) continue;

		// ni demasiado cerca de otro arbol
		bool tooClose = false;
		for (const glm::ivec2& p : placed)
			if (std::abs(x - p.x) + std::abs(z - p.y) < 5) tooClose = true; //	si la distancia Manhattan es menor a 5, consideramos que esta demasiado cerca
		if (tooClose) continue;

		PlaceTree(x, z); // colocamos el arbol en la posicion (x, z)
		placed.push_back(glm::ivec2(x, z));
	}
}