#version 330 core // Indicamos la versión de GLSL que vamos a utilizar

out vec4 FragColor; // Variable que contiene el color final del fragmento

in vec3 color; // Recibimos el color enviado desde el vertex shader

void main()
{
    // Asignamos el color recibido al fragmento
    // El cuarto valor representa la transparencia y 1.0 significa totalmente opaco
    FragColor = vec4(color, 1.0);
}