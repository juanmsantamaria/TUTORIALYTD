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
	static const int SIZE_X = 64;
	static const int SIZE_Y = 16;
	static const int SIZE_Z = 64;

	static const int HOUSE2_X = 48; // esquina de la segunda casa (al lado de la parcela)
	static const int HOUSE2_Z = 23;

	// altura de la capa de pasto (el suelo)
	static const int SURFACE_Y = 3;

	// tamaño de la casa (en bloques) y zonas reservadas del mapa
	static const int HOUSE_W = 7;  // ancho de la casa (eje X)
	static const int HOUSE_D = 7;  // fondo de la casa (eje Z)
	static const int HOUSE_X = 4;  // esquina de la casa de muestra
	static const int HOUSE_Z = 4;
	static const int PLOT_W = 25;  // ancho de la parcela (eje X)
	static const int PLOT_D = 15;  // fondo de la parcela (eje Z)
	static const int PLOT_X = 18;  // esquina de la parcela vacia donde se construye la copia
	static const int PLOT_Z = 19;

	World();

	// genera un terreno plano: piedra, dos capas de tierra y pasto arriba
	void GenerateFlat();

	// construye una casa de 7 x 7 con su esquina en (ox, oy, oz); oy es la altura del piso
	void PlaceHouse(int ox, int oy, int oz);

	// marca con un marco de piedra el terreno donde se construye la copia de la casa
	void MarkPlot(int ox, int oz);

	// planta un arbol en (x, z), sobre el pasto
	void PlaceTree(int x, int z);

	// planta varios arboles en posiciones pseudoaleatorias (la misma semilla da siempre el mismo bosque)
	void PlaceTrees(int count, unsigned int seed);

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