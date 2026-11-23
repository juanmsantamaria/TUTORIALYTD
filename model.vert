#version 330 core // Indicamos la versión de GLSL que vamos a utilizar

layout (location = 0) in vec3 aPos; // posición del vértice
layout (location = 1) in vec3 aNormal; // normal del vértice
layout (location = 2) in vec2 aTex; // coordenadas de textura
layout (location = 3) in vec4 aJoints; // los 4 huesos que mueven este vértice
layout (location = 4) in vec4 aWeights; // cuánto lo mueve cada hueso

out vec3 crntPos; // posición actual en el mundo
out vec3 normal; // normal en el mundo
out vec2 texCoord; // coordenadas de textura

uniform mat4 camMatrix; // matriz de la cámara (la envía camera.updateMatrix)
uniform mat4 model; // dónde está el modelo en el mundo
uniform mat4 nodeMatrix; // transformación del nodo (solo para piezas sin esqueleto)
uniform bool skinned; // true si el modelo se mueve con huesos

const int MAX_JOINTS = 128; // tiene que ser igual a MODEL_MAX_JOINTS de Model.h
uniform mat4 joints[MAX_JOINTS]; // matriz de cada hueso

void main()
{
	mat4 local;
	if (skinned)
	{
		// mezclamos los huesos que afectan a este vértice según su peso
		local = aWeights.x * joints[int(aJoints.x)]
		      + aWeights.y * joints[int(aJoints.y)]
		      + aWeights.z * joints[int(aJoints.z)]
		      + aWeights.w * joints[int(aJoints.w)];
	}
	else
	{
		local = nodeMatrix;
	}

	mat4 world = model * local;
	vec4 worldPos = world * vec4(aPos, 1.0f);

	crntPos = worldPos.xyz;
	normal = mat3(world) * aNormal;
	texCoord = aTex;

	gl_Position = camMatrix * worldPos;
}
