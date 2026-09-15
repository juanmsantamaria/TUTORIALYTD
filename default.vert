#version 330 core // Indicamos la versión de GLSL que vamos a utilizar

layout (location = 0) in vec3 aPos; // Recibe la posición del vértice
layout (location = 1) in vec3 aColor; // Recibe el color del vértice

layout (location = 2) in vec2 aTexCoord; // Recibe las coordenadas de textura del vértice

out vec3 color; // Variable que enviamos al fragment shader

out vec2 texCoord; // Variable que enviamos al fragment shader

uniform float scale; // Variable que permite modificar el tamaño del objeto

void main()
{
    // Calculamos la nueva posición del vértice aplicando la escala
    gl_Position = vec4(aPos + aPos * scale, 1.0);

    // Enviamos el color del vértice al fragment shader
    color = aColor;
    texCoord = aTexCoord;

}


