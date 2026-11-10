#include"CAMERA.h"

Camera::Camera(int width, int height, glm::vec3 startPosition)
{
	Camera::width = width;
	Camera::height = height; //|
	Posicion = startPosition; // antes: posicion = posicion (se asignaba a sí misma)
}

void Camera::updateMatrix(float FOVdeg, float nearPlane, float farPlane, Shader& shader, const char* uniform) // actualiza la matriz de proyección y vista de la cámara
{
	glm::mat4 cameraVista = glm::lookAt(Posicion, Posicion + orientacion, arriba); // crea la matriz de vista de la cámara usando la posición, orientación y vector hacia arriba
	glm::mat4 cameraProjection = glm::perspective(glm::radians(FOVdeg), (float)width / (float)height, nearPlane, farPlane); // crea la matriz de proyección de la cámara usando el campo de visión, la relación de aspecto y los planos cercano y lejano

	cameraMatrix = cameraProjection * cameraVista; // combina las matrices de proyección y vista

	glUniformMatrix4fv(glGetUniformLocation(shader.ID, uniform), 1, GL_FALSE, glm::value_ptr(cameraMatrix)); // envía la matriz de proyección y vista combinada al shader
}

void Camera::Inputs(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		Posicion += speed * orientacion;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		Posicion += speed * -glm::normalize(glm::cross(orientacion, arriba)); // cruzamos la orientación con el vector hacia arriba para obtener el vector lateral
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) 
	{
		Posicion += speed * -orientacion; // si se presiona la tecla S, la cámara se mueve hacia atrás
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		Posicion += speed * glm::normalize(glm::cross(orientacion, arriba)); // cruzamos la orientación con el vector hacia arriba para obtener el vector lateral
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) // si se presiona la tecla espacio, la cámara se mueve hacia arriba
	{
		Posicion += speed * arriba;
	} 
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)  // si se presiona la tecla control, la cámara se mueve hacia abajo
	{
		Posicion += speed * -arriba;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) // si se presiona la tecla shift, la velocidad de movimiento aumenta
	{
		speed = 0.1f;
	}
	else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
	{
		speed = 0.02f;
	}

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) // si se presiona el botón izquierdo del ratón, la cámara se mueve según el movimiento del ratón
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN); // ocultamos el cursor del ratón

		if (firstClick)
		{
			glfwSetCursorPos(window, (width / 2), (height / 2)); // colocamos el cursor en el centro de la ventana
			firstClick = false;
		}
		// obtenemos la posición del cursor del ratón
		double mouseX; 
		double mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);

		float rotX = sensitivity * (float)(mouseY - (height / 2)) / height; // calculamos la rotación en el eje Y según el movimiento del ratón
		float rotY = sensitivity * (float)(mouseX - (width / 2)) / width;

		glm::vec3 newOrientacion = glm::rotate(orientacion, glm::radians(-rotX), glm::normalize(glm::cross(orientacion, arriba))); // rotamos la orientación de la cámara según el movimiento del ratón

		// Evita que la cámara dé la vuelta completa (límite de 5° arriba/abajo)
		if (!(glm::angle(newOrientacion, arriba) <= glm::radians(5.0f) || glm::angle(newOrientacion, -arriba) <= glm::radians(5.0f))) // si la nueva orientación no está demasiado cerca del vector hacia arriba o hacia abajo
		{
			orientacion = newOrientacion; // actualizamos la orientación de la cámara
		}

		orientacion = glm::rotate(orientacion, glm::radians(-rotY), arriba); // rotamos la orientación de la cámara según el movimiento del ratón

		glfwSetCursorPos(window, (width / 2), (height / 2)); // colocamos el cursor en el centro de la ventana para que no se salga de la pantalla
	} 
	else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) // si se suelta el botón izquierdo del ratón, mostramos el cursor del ratón
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // mostramos el cursor del ratón
		firstClick = true;
	}
}
