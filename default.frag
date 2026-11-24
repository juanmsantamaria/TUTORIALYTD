#version 330 core // Indicamos la versión de GLSL que vamos a utilizar

out vec4 FragColor; // Variable que contiene el color final del fragmento

in vec3 crntPos; // Recibimos la posición actual enviada desde el vertex shader

in vec3 normal; // Recibimos la normal enviada desde el vertex shader

in vec3 color; // Recibimos el color enviado desde el vertex shader

in vec2 texCoord; // Recibimos las coordenadas de textura enviadas desde el vertex shader



uniform sampler2D diffuse0; // Declaramos la textura que vamos a utilizar

uniform sampler2D specular0; // Declaramos la textura que vamos a utilizar

uniform vec4 lightColor; // Declaramos el color de la luz que vamos a utilizar
uniform vec3 lightPos; // Declaramos la posición de la luz (main.cpp la envía con glUniform4f)
uniform vec3 camPos; // Declaramos la posición de la cámara (main.cpp la envía con glUniform3f)

uniform vec3 witherPos;
uniform float infectionRadius;
uniform float infectionTime;

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



	// Asignamos el color recibido al fragmento (la textura se tiñe con el color del bloque)
	FragColor = (texture(diffuse0, texCoord) * vec4(color, 1.0f) * (ambient + diffuse) + texture(specular0, texCoord).r * specular) * lightColor;

	//generamos una frontera irregular de la infeccion
float variation = sin(crntPos.x * 0.35f) * 2.0f
                + sin(crntPos.x * 0.9f) * 0.8f;

//limite fijo de la zona infectada
float infectionLimit = 40.0f + variation;

//transicion entre el terreno normal y el infectado
float infection = smoothstep(
    infectionLimit - 2.0f,
    infectionLimit + 2.0f,
    crntPos.z
);

//desaturamos los colores originales
float gray = dot(FragColor.rgb, vec3(0.299f, 0.587f, 0.114f));

//generamos un color oscuro con tonalidades moradas
vec3 corruptedColor = vec3(gray) * vec3(0.45f, 0.28f, 0.60f);

//mezclamos el color original con el infectado
FragColor.rgb = mix(FragColor.rgb, corruptedColor, infection);;
}