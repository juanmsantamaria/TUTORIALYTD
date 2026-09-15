#version 330 core // Indicamos la versión de GLSL que vamos a utilizar

layout (location = 0) in vec3 aPos; // Recibe la posición del vértice
layout (location = 1) in vec3 aColor; // Recibe el color del vértice

layout (location = 2) in vec2 aTexCoord; // Recibe las coordenadas de textura del vértice

out vec3 color; // Variable que enviamos al fragment shader

out vec2 texCoord; // Variable que enviamos al fragment shader

uniform float scale; // Variable que permite modificar el tamaño del objeto

uniform mat4 model; // Matriz de transformación del modelo
uniform mat4 view; // Matriz de transformación de la cámara
uniform mat4 proj; // Matriz de proyección


void main()
{
    // Calculamos la nueva posición del vértice aplicando la escala
    gl_Position = proj * view * model * vec4(aPos, 1.0);

    // Enviamos el color del vértice al fragment shader
    color = aColor;
    texCoord = aTexCoord;

}