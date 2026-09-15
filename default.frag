
#version 330 core // Indicamos la versión de GLSL que vamos a utilizar

out vec4 FragColor; // Variable que contiene el color final del fragmento

in vec2 texCoord; // Recibimos las coordenadas de textura enviadas desde el vertex shader

uniform sampler2D tex0; // Declaramos la textura que vamos a utilizar

void main()
{
// Asignamos el color recibido al fragmento
    FragColor = texture(tex0, texCoord);
}
