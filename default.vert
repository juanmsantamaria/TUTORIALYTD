#version 330 core // Indicamos la versión de GLSL que vamos a utilizar

layout (location = 0) in vec3 aPos; // Recibe la posición del vértice
layout (location = 1) in vec3 aColor; // Recibe el color del vértice

layout (location = 2) in vec2 aTexCoord; // Recibe las coordenadas de textura del vértice

layout (location = 3) in vec3 aNormal; // Recibe la normal del vértice

out vec3 color; // Variable que enviamos al fragment shader

out vec2 texCoord; // Variable que enviamos al fragment shader

out vec3 normal; // Variable que enviamos al fragment shader
out vec3 crntPos; // Variable que enviamos al fragment shader

uniform mat4 camMatrix; // Matriz de transformación del modelo
uniform mat4 model; // Matriz de transformación del modelo

void main()
{
crntPos = vec3(model * vec4(aPos, 1.0)); // Calculamos la posición del vértice en el espacio del mundo

    // Calculamos la nueva posición del vértice aplicando la escala
    gl_Position = camMatrix * vec4(crntPos, 1.0);

    // Enviamos el color del vértice al fragment shader
    color = aColor;
    texCoord = aTexCoord;
    normal = mat3(transpose(inverse(model))) * aNormal; // Calculamos la normal en el espacio del mundo

}