#include"World.h"
#include<cstring>
#include<cfloat>
#include<cstdlib>

// una cara del cubo: direccion del vecino, normal y las 4 esquinas (en sentido antihorario visto desde afuera)
struct Face
{
	int dx, dy, dz;
	glm::vec3 normal;
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
	glm::vec2(0.0f, 0.0f),
	glm::vec2(1.0f, 0.0f),
	glm::vec2(1.0f, 1.0f),
	glm::vec2(0.0f, 1.0f)
};

// color con el que se tiñe la textura de cada tipo de bloque (provisional, hasta tener un atlas)
static glm::vec3 BlockColor(uint8_t type)


{




	switch (type)
	{
	case GRASS:  return glm::vec3(0.55f, 0.90f, 0.35f);
	case DIRT:   return glm::vec3(0.85f, 0.60f, 0.40f);
	case STONE:  return glm::vec3(0.70f, 0.70f, 0.72f);
	case WOOD:   return glm::vec3(0.60f, 0.42f, 0.25f);
	case LEAVES: return glm::vec3(0.25f, 0.75f, 0.25f);
	case PLANKS: return glm::vec3(1.00f, 0.85f, 0.55f);
	default:     return glm::vec3(1.0f, 1.0f, 1.0f);
	}
}

World::World()
{
	std::memset(blocks, AIR, sizeof(blocks));
}

void World::GenerateFlat()
{
	for (int x = 0; x < SIZE_X; x++)
	{
		for (int z = 0; z < SIZE_Z; z++)
		{
			blocks[x][0][z] = STONE;
			blocks[x][1][z] = DIRT;
			blocks[x][2][z] = DIRT;
			blocks[x][3][z] = GRASS;
		}
	}
}

uint8_t World::GetBlock(int x, int y, int z) const
{
	if (x < 0 || x >= SIZE_X || y < 0 || y >= SIZE_Y || z < 0 || z >= SIZE_Z)
		return AIR;
	return blocks[x][y][z];
}

void World::SetBlock(int x, int y, int z, uint8_t type)
{
	if (x < 0 || x >= SIZE_X || y < 0 || y >= SIZE_Y || z < 0 || z >= SIZE_Z)
		return;
	blocks[x][y][z] = type;
}

bool World::IsOpaque(int x, int y, int z) const
{
	if (y < 0) return true; // debajo del mundo: no dibujamos la cara de abajo
	return GetBlock(x, y, z) != AIR;
}

void World::BuildMesh(std::vector<Vertex>& vertices, std::vector<GLuint>& indices) const
{
	vertices.clear();
	indices.clear();

	for (int x = 0; x < SIZE_X; x++)
	{
		for (int y = 0; y < SIZE_Y; y++)
		{
			for (int z = 0; z < SIZE_Z; z++)
			{
				uint8_t type = blocks[x][y][z];
				if (type == AIR) continue;

				glm::vec3 color = BlockColor(type);

				for (int f = 0; f < 6; f++)
				{
					const Face& face = FACES[f];

					// si el vecino tapa esta cara, no la dibujamos
					if (IsOpaque(x + face.dx, y + face.dy, z + face.dz)) continue;

					GLuint base = (GLuint)vertices.size();

					for (int c = 0; c < 4; c++)
					{
						glm::vec3 pos = glm::vec3((float)x, (float)y, (float)z) + face.corners[c];
						vertices.push_back(Vertex{ pos, face.normal, color, UVS[c] });
					}

					indices.push_back(base);
					indices.push_back(base + 1);
					indices.push_back(base + 2);
					indices.push_back(base);
					indices.push_back(base + 2);
					indices.push_back(base + 3);
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

	for (int i = 0; i < 3; i++)
	{
		if (dir[i] > 0.0f)
		{
			step[i] = 1;
			tMax[i] = ((float)(cell[i] + 1) - origin[i]) / dir[i];
			tDelta[i] = 1.0f / dir[i];
		}
		else if (dir[i] < 0.0f)
		{
			step[i] = -1;
			tMax[i] = (origin[i] - (float)cell[i]) / -dir[i];
			tDelta[i] = 1.0f / -dir[i];
		}
	}

	float t = 0.0f;
	while (t <= maxDistance)
	{
		if (GetBlock(cell.x, cell.y, cell.z) != AIR)
		{
			hit = cell;
			previous = prevCell;
			return true;
		}

		prevCell = cell;

		// avanzamos por el eje cuyo borde esta mas cerca
		int axis;
		if (tMax.x < tMax.y)
			axis = (tMax.x < tMax.z) ? 0 : 2;
		else
			axis = (tMax.y < tMax.z) ? 1 : 2;

		t = tMax[axis];
		cell[axis] += step[axis];
		tMax[axis] += tDelta[axis];
	}

	return false;
}

// true si (x, z) cae dentro del area de una casa (mas un margen)
static bool InsideArea(int x, int z, int ox, int oz, int margin)
{
	return x >= ox - margin && x < ox + World::HOUSE_W + margin &&
		z >= oz - margin && z < oz + World::HOUSE_D + margin;
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
	// marco de piedra en el suelo, justo afuera del area de 7 x 7
	for (int x = -1; x <= HOUSE_W; x++)
		for (int z = -1; z <= HOUSE_D; z++)
			if (x == -1 || x == HOUSE_W || z == -1 || z == HOUSE_D)
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
		for (int dx = -r; dx <= r; dx++)
		{
			for (int dz = -r; dz <= r; dz++)
			{
				bool corner = (std::abs(dx) == r && std::abs(dz) == r);
				if (corner && (dy <= 0 || dy == 2)) continue;
				if (GetBlock(x + dx, top + dy, z + dz) == AIR)
					SetBlock(x + dx, top + dy, z + dz, LEAVES);
			}
		}
	}

	// tronco
	for (int y = base; y <= top; y++)
		SetBlock(x, y, z, WOOD);
}

void World::PlaceTrees(int count, unsigned int seed)
{
	unsigned int state = seed;
	std::vector<glm::ivec2> placed;

	for (int attempt = 0; attempt < 500 && (int)placed.size() < count; attempt++)
	{
		// generador de numeros pseudoaleatorios sencillo (misma semilla = mismo resultado)
		state = state * 1664525u + 1013904223u;
		int x = 3 + (int)((state >> 16) % (unsigned int)(SIZE_X - 6));
		state = state * 1664525u + 1013904223u;
		int z = 3 + (int)((state >> 16) % (unsigned int)(SIZE_Z - 6));

		// no plantamos sobre la casa ni sobre la parcela
		if (InsideArea(x, z, HOUSE_X, HOUSE_Z, 3)) continue;
		if (InsideArea(x, z, PLOT_X, PLOT_Z, 3)) continue;

		// ni demasiado cerca de otro arbol
		bool tooClose = false;
		for (const glm::ivec2& p : placed)
			if (std::abs(x - p.x) + std::abs(z - p.y) < 5) tooClose = true;
		if (tooClose) continue;

		PlaceTree(x, z);
		placed.push_back(glm::ivec2(x, z));
	}
}