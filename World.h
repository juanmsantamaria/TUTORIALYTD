#ifndef WORLD_CLASS_H
#define WORLD_CLASS_H

#include<vector>
#include<cstdint>
#include<glm/glm.hpp>
#include"VBO.h" // aqui esta definido el struct Vertex

// tipos de bloque (0 = aire, no se dibuja)
enum BlockType : uint8_t
{
	AIR = 0,
	GRASS,
	DIRT,
	STONE,
	WOOD,
	LEAVES,
	PLANKS
};

class World
{
public:
	// tamaño del mundo en bloques (cada bloque mide 1 x 1 x 1)
	static const int SIZE_X = 32;
	static const int SIZE_Y = 16;
	static const int SIZE_Z = 32;

	World();

	// genera un terreno plano: piedra, dos capas de tierra y pasto arriba
	void GenerateFlat();

	// devuelve el tipo de bloque (AIR si la posicion esta fuera del mundo)
	uint8_t GetBlock(int x, int y, int z) const;

	// cambia un bloque (no hace nada si la posicion esta fuera del mundo)
	void SetBlock(int x, int y, int z, uint8_t type);

	// genera la malla del mundo, solo con las caras visibles
	void BuildMesh(std::vector<Vertex>& vertices, std::vector<GLuint>& indices) const;

	// lanza un rayo y devuelve true si toca un bloque.
	// hit = bloque tocado, previous = celda vacia justo antes (donde se coloca un bloque nuevo)
	bool Raycast(glm::vec3 origin, glm::vec3 direction, float maxDistance, glm::ivec3& hit, glm::ivec3& previous) const;

private:
	uint8_t blocks[SIZE_X][SIZE_Y][SIZE_Z];

	// true si el bloque tapa a su vecino (se usa para no dibujar caras ocultas)
	bool IsOpaque(int x, int y, int z) const;
};

#endif