#ifndef MODEL_CLASS_H
#define MODEL_CLASS_H

#include<glad/glad.h>
#include<glm/glm.hpp>
#include<glm/gtc/quaternion.hpp>
#include<string>
#include<vector>
#include"Camera.h" // de aqui viene la clase Shader

// maximo de huesos que acepta model.vert (tiene que ser igual a MAX_JOINTS del shader)
#define MODEL_MAX_JOINTS 128

// un vertice del modelo: posicion, normal, uv, 4 huesos que lo mueven y el peso de cada uno
struct ModelVertex
{
	float position[3];
	float normal[3];
	float texUV[2];
	float joints[4];
	float weights[4];
};

// un pedazo de malla que se dibuja con un solo material
struct ModelPrimitive
{
	GLuint vao, vbo, ebo;
	GLsizei indexCount;
	GLuint texture;       // textura de color (si no tiene, una blanca de 1 pixel)
	glm::vec4 baseColor;  // color que multiplica la textura
	float alphaCutoff;    // los pixeles con menos alfa que esto se descartan (alas, hojas...)
	bool doubleSided;     // se ve por los dos lados
	int node;             // nodo al que pertenece
	int skin;             // esqueleto que la mueve (-1 si no tiene)
};

// un nodo (hueso o pieza) del modelo
struct ModelNode
{
	int parent;
	std::vector<int> children;
	glm::vec3 translation; // pose de reposo
	glm::quat rotation;
	glm::vec3 scale;
	glm::mat4 restLocal;   // matriz de reposo (por si el nodo trae "matrix")
	bool animated;         // true si alguna animacion lo mueve
	glm::mat4 world;       // matriz final del nodo (se recalcula en Update)
};

// esqueleto: lista de nodos-hueso y su matriz inversa de reposo
struct ModelSkin
{
	std::vector<int> joints;
	std::vector<glm::mat4> inverseBind;
	std::vector<glm::mat4> jointMatrices; // se recalcula en Update
};

// una animacion = varios canales, cada uno mueve la posicion, rotacion o escala de un nodo
struct ModelChannel
{
	int node;
	int path;   // 0 = posicion, 1 = rotacion, 2 = escala
	int interp; // 0 = lineal, 1 = escalon, 2 = cubica (se aproxima como lineal)
	std::vector<float> times;
	std::vector<float> values;
};

struct ModelAnimation
{
	std::string name;
	float duration;
	std::vector<ModelChannel> channels;
};

class Model
{
public:
	Model() {}

	// carga un .glb o .gltf. targetHeight = alto que tendra el modelo en el mundo (en bloques)
	bool Load(const char* file, float targetHeight);

	// calcula la pose del modelo en el segundo "time" de la animacion "animation" (-1 = pose de reposo)
	void Update(float time, int animation = 0);

	// dibuja el modelo; "matrix" lo coloca, gira y escala en el mundo (los pies quedan en y = 0)
	void Draw(Shader& shader, glm::mat4 matrix);

	// libera la memoria de la GPU
	void Delete();

	int AnimationCount() const { return (int)animations.size(); }
	std::string AnimationName(int i) const { return animations[i].name; }

private:
	std::vector<ModelNode> nodes;
	std::vector<ModelSkin> skins;
	std::vector<ModelAnimation> animations;
	std::vector<ModelPrimitive> primitives;
	std::vector<GLuint> textures;
	glm::mat4 normMatrix = glm::mat4(1.0f); // escala el modelo a targetHeight y le pone los pies en y = 0

	void UpdateNode(int i, const glm::mat4& parentWorld, const std::vector<glm::mat4>& local);
};

#endif
