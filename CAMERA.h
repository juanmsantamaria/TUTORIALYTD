#ifndef CAMERA_CLASS_H
#define CAMERA_CLASS_H

#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include<glm/gtx/rotate_vector.hpp>
#include<glm/gtx/vector_angle.hpp>

#include"shaderClass.h"

class Camera
{
public:
	glm::vec3 Posicion; // Posición de la cámara en el espacio 3D
	glm::vec3 orientacion = glm::vec3(0.0f, 0.0f, -1.0f); // Dirección hacia la que la cámara está mirando
	glm::vec3 arriba = glm::vec3(0.0f, 1.0f, 0.0f); // Vector que representa la dirección hacia arriba de la cámara
	glm::mat4 cameraMatrix = glm::mat4(1.0f); // Matriz de la cámara que combina la proyección y la vista

	bool firstClick = true; 

	int width; 
	int height; 

	float speed = 0.1f; // Velocidad de movimiento de la cámara
	float sensitivity = 100.0f; // Sensibilidad del ratón para la rotación de la cámara

	// El parámetro tiene un nombre distinto al miembro para evitar "x = x"
	Camera(int width, int height, glm::vec3 startPosition); // Constructor de la clase Camera, recibe el ancho y alto de la ventana y la posición inicial de la cámara

	void updateMatrix(float FOVdeg, float nearPlane, float farPlane, Shader& shader, const char* uniform); // Actualiza la matriz de proyección y vista de la cámara y la envía al shader
	

	void Inputs(GLFWwindow* window); // Maneja la entrada del usuario para mover y rotar la cámara según las teclas presionadas y el movimiento del ratón

};

#endif