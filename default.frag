#version 330 core // Indicamos la versión de GLSL que vamos a utilizar

out vec4 FragColor; // Variable que contiene el color final del fragmento

in vec3 color; // Recibimos el color enviado desde el vertex shader
in vec2 texCoord; // Recibimos las coordenadas de textura enviadas desde el vertex shader
in vec3 normal; // Recibimos la normal enviada desde el vertex shader
in vec3 crntPos; // Recibimos la posición actual enviada desde el vertex shader

uniform sampler2D tex0; // Declaramos la textura que vamos a utilizar

uniform sampler2D tex1; // Declaramos la textura que vamos a utilizar

uniform vec4 lightColor; // Declaramos el color de la luz que vamos a utilizar
uniform vec3 lightPos; // Declaramos la posición de la luz (main.cpp la envía con glUniform4f)
uniform vec3 camPos; // Declaramos la posición de la cámara (main.cpp la envía con glUniform3f)
void main()
{
    float ambient = 0.2f; // Definimos la intensidad de la luz ambiental

	vec3 normalizedNormal = normalize(normal); // Normalizamos la normal
	vec3 lightDir = normalize(lightPos.xyz - crntPos); // Dirección desde el fragmento hacia la luz

	float diffuse = max(dot(normalizedNormal, lightDir), 0.0f); // Calculamos la intensidad de la luz utilizando el producto punto entre la normal y la dirección de la luz

	float specularLight = 0.5f; // 
	vec3 viewDirection = normalize(camPos - crntPos); // Dirección desde el fragmento hacia la cámara
	vec3 reflectDirection = reflect(-lightDir, normalizedNormal); // Dirección de reflexión de la luz
	float specAmount = pow(max(dot(viewDirection, reflectDirection), 0.0f), 16); // Calculamos la intensidad de la luz especular utilizando el producto punto entre la dirección de la cámara y la dirección de reflexión de la luz
	float specular = specularLight * specAmount; // Calculamos la intensidad de la luz especular



	// Asignamos el color recibido al fragmento
	FragColor = (texture(tex0, texCoord) * (ambient + diffuse) + texture(tex1, texCoord).r * specular) * lightColor;
}