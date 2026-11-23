#include"World.h"
#include<cstring>
#include<cfloat>

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