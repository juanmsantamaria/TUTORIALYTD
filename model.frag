#version 330 core // Indicamos la versión de GLSL que vamos a utilizar

out vec4 FragColor; // Variable que contiene el color final del fragmento

in vec3 crntPos; // posición en el mundo
in vec3 normal; // normal en el mundo
in vec2 texCoord; // coordenadas de textura

uniform sampler2D diffuse0; // textura de color del modelo
uniform vec4 baseColor; // color que multiplica la textura
uniform float alphaCutoff; // los pixeles con menos alfa que esto se descartan (alas, etc.)
uniform vec4 lightColor; // color de la luz
uniform vec3 lightPos; // posición de la luz

void main()
{
	vec4 texColor = texture(diffuse0, texCoord) * baseColor;
	if (texColor.a < alphaCutoff)
		discard; // pixel transparente

	vec3 n = normalize(normal);
	if (!gl_FrontFacing)
		n = -n; // si vemos la cara de atrás de una pieza plana, invertimos la normal

	float ambient = 0.3f;
	vec3 lightDir = normalize(lightPos - crntPos);
	float diffuse = max(dot(n, lightDir), 0.0f);

	FragColor = vec4(texColor.rgb * (ambient + diffuse), 1.0f) * lightColor;
}
