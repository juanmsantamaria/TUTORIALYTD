#include<stb/stb_image.h> // solo las declaraciones: la implementacion de stb_image ya esta en tu proyecto (la usa Texture.cpp)

// tinygltf (con su json.hpp) lee el archivo .gltf/.glb. Le decimos que no incluya su propia copia de stb_image
#define TINYGLTF_IMPLEMENTATION // le dice a tinygltf que compile aqui su codigo (solo se hace en un .cpp)
#define TINYGLTF_NO_INCLUDE_STB_IMAGE // no incluye su propio stb_image (ya tenemos el nuestro)
#define TINYGLTF_NO_STB_IMAGE_WRITE // no necesitamos guardar imagenes, solo leerlas
#include"tiny_gltf.h" // la libreria que lee archivos glTF (usa json.hpp por dentro)

#include"Model.h" // la declaracion de la clase Model y sus structs
#include<glm/gtc/type_ptr.hpp> // para pasar matrices de glm a OpenGL (value_ptr) y crearlas desde arrays (make_mat4)
#include<glm/gtc/matrix_transform.hpp> // translate, scale, rotate
#include<iostream> // para escribir mensajes en la consola
#include<algorithm> // std::max
#include<cmath> // std::fmod
#include<cfloat> // FLT_MAX
#include<cstring> // std::memcpy y std::memset
#include<cstdint> // tipos de entero de tamaño fijo (int8_t, uint16_t...)
#include<map> // std::map, para guardar las texturas ya cargadas


// funciones de ayuda


// crea una textura de OpenGL de 1 pixel (se usa cuando el material no tiene imagen)
static GLuint SolidTexture(unsigned char r, unsigned char g, unsigned char b)
{
	unsigned char pixel[4] = { r, g, b, 255 }; // el unico pixel de la textura: color RGB y alfa 255 (opaco)
	GLuint id; // numero con el que OpenGL identifica la textura
	glGenTextures(1, &id); // pedimos a OpenGL que cree una textura
	glActiveTexture(GL_TEXTURE0); // usamos la unidad de textura 0
	glBindTexture(GL_TEXTURE_2D, id); // la dejamos activa para configurarla
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // filtro al verla pequeña: sin suavizar
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // filtro al verla grande: sin suavizar
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel); // subimos el pixel a la GPU (imagen de 1 x 1)
	return id; // devolvemos el numero de la textura
}

// lee un numero de un accessor segun su tipo (float, byte, short...), normalizandolo si hace falta
static float ReadComponent(const unsigned char* p, int type, bool normalized)
{
	switch (type) // segun como este guardado el numero en el archivo
	{
	case TINYGLTF_COMPONENT_TYPE_FLOAT: // numero decimal de 4 bytes
	{
		float v; std::memcpy(&v, p, 4); return v; // copiamos los 4 bytes a un float y lo devolvemos
	}
	case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: // entero de 1 byte, de 0 a 255
	{
		float v = (float)(*p); return normalized ? v / 255.0f : v; // si es "normalizado" lo pasamos al rango 0 a 1
	}
	case TINYGLTF_COMPONENT_TYPE_BYTE: // entero de 1 byte con signo, de -128 a 127
	{
		float v = (float)(*(const int8_t*)p); return normalized ? std::max(v / 127.0f, -1.0f) : v; // si es normalizado, rango -1 a 1
	}
	case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: // entero de 2 bytes, de 0 a 65535
	{
		uint16_t v; std::memcpy(&v, p, 2); return normalized ? v / 65535.0f : (float)v; // si es normalizado, rango 0 a 1
	}
	case TINYGLTF_COMPONENT_TYPE_SHORT: // entero de 2 bytes con signo
	{
		int16_t v; std::memcpy(&v, p, 2); return normalized ? std::max(v / 32767.0f, -1.0f) : (float)v; // si es normalizado, rango -1 a 1
	}
	case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: // entero de 4 bytes (indices de modelos grandes)
	{
		uint32_t v; std::memcpy(&v, p, 4); return (float)v; // lo pasamos a float
	}
	}
	return 0.0f; // tipo desconocido: devolvemos 0
}

// lee el elemento "index" de un accessor (por ejemplo, un vertice) y lo guarda en out (maximo n numeros)
static void ReadElement(const tinygltf::Model& g, int accessorIndex, size_t index, float* out, int n)
{
	for (int c = 0; c < n; c++) out[c] = 0.0f; // dejamos la salida en ceros por si no hay datos
	if (accessorIndex < 0) return; // el atributo no existe en el modelo

	const tinygltf::Accessor& a = g.accessors[accessorIndex]; // el accessor describe como leer un grupo de datos (posiciones, normales...)
	if (a.bufferView < 0) return; // sin datos: todo ceros

	const tinygltf::BufferView& bv = g.bufferViews[a.bufferView]; // trozo del buffer donde estan esos datos
	const tinygltf::Buffer& buf = g.buffers[bv.buffer]; // el buffer con todos los bytes del modelo

	int comps = tinygltf::GetNumComponentsInType(a.type); // cuantos numeros tiene cada elemento (vec3 = 3, vec4 = 4...)
	int compSize = tinygltf::GetComponentSizeInBytes(a.componentType); // cuantos bytes ocupa cada numero
	size_t stride = (size_t)a.ByteStride(bv); // bytes que hay entre un elemento y el siguiente
	if (stride == 0) stride = (size_t)(comps * compSize); // si no lo indica, los elementos estan pegados uno tras otro

	const unsigned char* p = buf.data.data() + bv.byteOffset + a.byteOffset + index * stride; // direccion en memoria del elemento que queremos
	for (int c = 0; c < comps && c < n; c++) // leemos cada numero del elemento (sin pasarnos de n)
		out[c] = ReadComponent(p + c * compSize, a.componentType, a.normalized);
}

// carga una textura del glTF a OpenGL (tinygltf ya decodifico la imagen)
static GLuint LoadGltfTexture(const tinygltf::Model& g, int textureIndex, std::map<int, GLuint>& cache, std::vector<GLuint>& created, GLuint white)
{
	if (textureIndex < 0 || textureIndex >= (int)g.textures.size()) return white; // el material no tiene textura: usamos la blanca

	auto found = cache.find(textureIndex); // revisamos si esta textura ya se subio antes
	if (found != cache.end()) return found->second; // si ya existe, la reutilizamos

	const tinygltf::Texture& tex = g.textures[textureIndex]; // datos de la textura (que imagen usa y que sampler)
	GLuint id; // numero de la textura en OpenGL

	// si no hay imagen, esta vacia o no es de 8 bits por canal, no la podemos usar
	if (tex.source < 0 || tex.source >= (int)g.images.size() || g.images[tex.source].image.empty() || g.images[tex.source].bits != 8)
	{
		std::cout << "No se pudo cargar una textura del modelo (" << (tex.source >= 0 && tex.source < (int)g.images.size() ? g.images[tex.source].uri : std::string("sin imagen")) << ")" << std::endl; // avisamos en la consola
		id = SolidTexture(255, 0, 255); // magenta = falta la textura
		created.push_back(id); // la anotamos para borrarla al final
		cache[textureIndex] = id; // y la guardamos para no repetir el error
		return id;
	}

	const tinygltf::Image& img = g.images[tex.source]; // la imagen ya decodificada por tinygltf

	// pasamos la imagen a RGBA sin importar cuantos canales traiga
	std::vector<unsigned char> rgba((size_t)img.width * img.height * 4); // 4 bytes (R, G, B, A) por cada pixel
	for (size_t i = 0; i < (size_t)img.width * img.height; i++) // recorremos todos los pixeles
	{
		const unsigned char* s = &img.image[i * img.component]; // pixel original
		unsigned char* d = &rgba[i * 4]; // pixel destino
		if (img.component == 1) { d[0] = d[1] = d[2] = s[0]; d[3] = 255; } // 1 canal (gris): lo copiamos a R, G y B
		else if (img.component == 2) { d[0] = d[1] = d[2] = s[0]; d[3] = s[1]; } // 2 canales (gris y alfa)
		else if (img.component == 3) { d[0] = s[0]; d[1] = s[1]; d[2] = s[2]; d[3] = 255; } // 3 canales (RGB): alfa opaco
		else { d[0] = s[0]; d[1] = s[1]; d[2] = s[2]; d[3] = s[3]; } // 4 canales (RGBA): copia directa
	}

	GLint wrapS = GL_REPEAT, wrapT = GL_REPEAT; // por defecto la textura se repite si las uv se salen de 0 a 1
	if (tex.sampler >= 0 && tex.sampler < (int)g.samplers.size()) // si el modelo indica otro modo de repeticion, lo usamos
	{
		wrapS = g.samplers[tex.sampler].wrapS; // modo horizontal
		wrapT = g.samplers[tex.sampler].wrapT; // modo vertical
	}

	glGenTextures(1, &id); // creamos la textura en OpenGL
	glActiveTexture(GL_TEXTURE0); // unidad de textura 0
	glBindTexture(GL_TEXTURE_2D, id); // la dejamos activa para configurarla
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR); // al verla pequeña (lejos) usa mipmaps
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // pixeles cuadrados, como en Minecraft
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS); // repeticion horizontal
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT); // repeticion vertical
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width, img.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba.data()); // subimos los pixeles a la GPU
	glGenerateMipmap(GL_TEXTURE_2D); // creamos las versiones reducidas de la imagen

	created.push_back(id); // la anotamos para borrarla al final
	cache[textureIndex] = id; // la guardamos por si otro material usa la misma
	return id;
}

// valor de un canal de animacion en el instante t
static void SampleChannel(const ModelChannel& c, float t, glm::vec3& position, glm::quat& rotation, glm::vec3& scale)
{
	size_t n = c.times.size(); // cantidad de claves (fotogramas) del canal
	if (n == 0) return; // canal vacio: no hacemos nada

	size_t comps = (c.path == 1) ? 4 : 3; // la rotacion usa 4 numeros (quaternion); posicion y escala usan 3
	size_t stride = (c.interp == 2) ? 3 : 1; // la cubica guarda 3 valores por clave: tangente entrada, valor y tangente salida
	size_t offset = (c.interp == 2) ? 1 : 0; // en la cubica el valor real es el del medio

	// buscamos entre que dos claves esta t
	size_t k0, k1; // clave anterior y clave siguiente
	float f = 0.0f; // cuanto avanzamos entre las dos claves (0 a 1)
	if (t <= c.times.front()) { k0 = k1 = 0; } // antes de la primera clave: nos quedamos en ella
	else if (t >= c.times.back()) { k0 = k1 = n - 1; } // despues de la ultima: nos quedamos en ella
	else
	{
		k1 = 1; // empezamos a buscar desde la segunda clave
		while (c.times[k1] < t) k1++; // avanzamos hasta la primera clave que ya pasa de t
		k0 = k1 - 1; // la clave anterior
		f = (t - c.times[k0]) / (c.times[k1] - c.times[k0]); // porcentaje del recorrido entre las dos claves
		if (c.interp == 1) f = 0.0f; // escalon: se queda con la clave anterior
	}

	float a[4] = { 0, 0, 0, 1 }, b[4] = { 0, 0, 0, 1 }; // valores de la clave anterior (a) y de la siguiente (b)
	for (size_t j = 0; j < comps; j++) // copiamos los numeros de cada clave desde el arreglo de valores
	{
		a[j] = c.values[(k0 * stride + offset) * comps + j];
		b[j] = c.values[(k1 * stride + offset) * comps + j];
	}

	if (c.path == 0) // el canal mueve la posicion
		position = glm::mix(glm::vec3(a[0], a[1], a[2]), glm::vec3(b[0], b[1], b[2]), f); // mezcla lineal entre las dos claves
	else if (c.path == 2) // el canal cambia la escala
		scale = glm::mix(glm::vec3(a[0], a[1], a[2]), glm::vec3(b[0], b[1], b[2]), f); // mezcla lineal entre las dos claves
	else // el canal gira el nodo
	{
		glm::quat qa(a[3], a[0], a[1], a[2]); // glTF guarda x, y, z, w; glm::quat recibe w, x, y, z
		glm::quat qb(b[3], b[0], b[1], b[2]);
		rotation = glm::normalize(glm::slerp(qa, qb, f)); // interpolacion esferica: gira suave entre las dos rotaciones
	}
}



// carga del modelo

bool Model::Load(const char* file, float targetHeight)
{
	tinygltf::TinyGLTF loader; // el objeto que sabe leer archivos glTF
	tinygltf::Model g; // aqui queda guardado todo el modelo leido (nodos, mallas, materiales...)
	std::string err, warn; // mensajes de error y de aviso

	std::string path = file; // ruta del archivo como texto
	bool binary = path.size() >= 4 && (path.substr(path.size() - 4) == ".glb" || path.substr(path.size() - 4) == ".GLB"); // true si el archivo termina en .glb

	stbi_set_flip_vertically_on_load(false); // en glTF la imagen ya viene con el origen arriba a la izquierda
	bool ok = binary ? loader.LoadBinaryFromFile(&g, &err, &warn, path) : loader.LoadASCIIFromFile(&g, &err, &warn, path); // leemos el .glb (binario) o el .gltf (texto)
	stbi_set_flip_vertically_on_load(true); // dejamos la opcion como la usa la clase Texture

	if (!warn.empty()) std::cout << "Aviso al leer el modelo: " << warn << std::endl; // mostramos los avisos si hubo
	if (!ok) // si no se pudo leer el archivo
	{
		std::cout << "No se pudo leer el modelo: " << file << " " << err << std::endl; // mostramos el error
		return false; // y salimos avisando que fallo
	}

	//  nodos
	nodes.resize(g.nodes.size()); // un nodo nuestro por cada nodo del modelo
	for (size_t i = 0; i < g.nodes.size(); i++) // recorremos todos los nodos
	{
		const tinygltf::Node& n = g.nodes[i]; // nodo original del archivo
		ModelNode& m = nodes[i]; // nuestro nodo, que vamos a rellenar

		m.parent = -1; // se completa abajo con los hijos de cada nodo
		m.children = n.children; // lista de hijos del nodo
		m.translation = glm::vec3(0.0f); // posicion de reposo (por defecto en el origen)
		m.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // rotacion de reposo (por defecto sin girar)
		m.scale = glm::vec3(1.0f); // escala de reposo (por defecto tamaño normal)
		if (n.translation.size() == 3) m.translation = glm::vec3((float)n.translation[0], (float)n.translation[1], (float)n.translation[2]); // si el nodo trae posicion, la usamos
		if (n.rotation.size() == 4) m.rotation = glm::quat((float)n.rotation[3], (float)n.rotation[0], (float)n.rotation[1], (float)n.rotation[2]); // glTF: x, y, z, w
		if (n.scale.size() == 3) m.scale = glm::vec3((float)n.scale[0], (float)n.scale[1], (float)n.scale[2]); // si el nodo trae escala, la usamos

		if (n.matrix.size() == 16) // algunos nodos traen directamente una matriz 4x4
		{
			float mm[16]; // matriz como lista de 16 numeros
			for (int k = 0; k < 16; k++) mm[k] = (float)n.matrix[k]; // pasamos de double a float
			m.restLocal = glm::make_mat4(mm); // la convertimos a matriz de glm
		}
		else
		{
			m.restLocal = glm::translate(glm::mat4(1.0f), m.translation) * glm::mat4_cast(m.rotation) * glm::scale(glm::mat4(1.0f), m.scale); // matriz = posicion * rotacion * escala
		}
		m.animated = false; // por ahora ninguna animacion lo mueve
		m.world = glm::mat4(1.0f); // matriz final (se calcula en Update)
	}
	for (size_t i = 0; i < nodes.size(); i++) // ahora que tenemos todos los nodos, anotamos el padre de cada uno
		for (size_t c = 0; c < nodes[i].children.size(); c++)
			nodes[nodes[i].children[c]].parent = (int)i; // el nodo i es padre de cada uno de sus hijos

	// esqueletos
	skins.resize(g.skins.size()); // un esqueleto nuestro por cada "skin" del modelo
	for (size_t s = 0; s < g.skins.size(); s++) // recorremos los esqueletos
	{
		const tinygltf::Skin& skin = g.skins[s]; // esqueleto original
		ModelSkin& sk = skins[s]; // nuestro esqueleto

		size_t jointCount = skin.joints.size(); // cantidad de huesos
		if (jointCount > MODEL_MAX_JOINTS) // si hay mas huesos de los que acepta el shader
		{
			std::cout << "Aviso: el modelo tiene " << jointCount << " huesos y solo se usan " << MODEL_MAX_JOINTS << " (sube MODEL_MAX_JOINTS en Model.h y MAX_JOINTS en model.vert)" << std::endl;
			jointCount = MODEL_MAX_JOINTS; // usamos solo los que caben
		}

		sk.joints.assign(skin.joints.begin(), skin.joints.begin() + jointCount); // lista de nodos que son huesos
		sk.inverseBind.assign(jointCount, glm::mat4(1.0f)); // matrices inversas de reposo (por defecto identidad)
		if (skin.inverseBindMatrices >= 0) // si el modelo las trae
		{
			for (size_t j = 0; j < jointCount; j++) // leemos la de cada hueso
			{
				float raw[16]; // los 16 numeros de la matriz
				ReadElement(g, skin.inverseBindMatrices, j, raw, 16);
				sk.inverseBind[j] = glm::make_mat4(raw); // la convertimos a matriz de glm
			}
		}
		sk.jointMatrices.assign(jointCount, glm::mat4(1.0f)); // matrices finales de cada hueso (se calculan en Update)
	}

	//  animaciones
	for (size_t a = 0; a < g.animations.size(); a++) // recorremos las animaciones del modelo
	{
		const tinygltf::Animation& anim = g.animations[a]; // animacion original
		ModelAnimation out; // nuestra animacion
		out.name = anim.name.empty() ? ("animacion " + std::to_string(a)) : anim.name; // nombre (o "animacion N" si no tiene)
		out.duration = 0.0f; // duracion, se calcula con los canales

		for (size_t c = 0; c < anim.channels.size(); c++) // cada canal mueve una propiedad de un nodo
		{
			const tinygltf::AnimationChannel& ch = anim.channels[c]; // canal original
			if (ch.target_node < 0 || ch.sampler < 0 || ch.sampler >= (int)anim.samplers.size()) continue; // canal invalido: lo saltamos
			const tinygltf::AnimationSampler& sampler = anim.samplers[ch.sampler]; // sampler: de donde salen los tiempos y los valores

			ModelChannel mc; // nuestro canal
			mc.node = ch.target_node; // nodo que mueve
			if (ch.target_path == "translation") mc.path = 0; // mueve la posicion
			else if (ch.target_path == "rotation") mc.path = 1; // gira
			else if (ch.target_path == "scale") mc.path = 2; // cambia la escala
			else continue; // (animaciones de "morph targets" no se usan)

			if (sampler.interpolation == "STEP") mc.interp = 1; // salta de una clave a otra sin suavizar
			else if (sampler.interpolation == "CUBICSPLINE") mc.interp = 2; // curva cubica
			else mc.interp = 0; // lineal (por defecto)

			size_t keys = (size_t)g.accessors[sampler.input].count; // cantidad de claves (fotogramas)
			size_t comps = (mc.path == 1) ? 4 : 3; // numeros por valor: 4 para rotacion, 3 para lo demas
			size_t perKey = (mc.interp == 2) ? 3 : 1; // valores guardados por clave (3 en la cubica)
			if (keys == 0 || (size_t)g.accessors[sampler.output].count < keys * perKey) continue; // canal danado, lo ignoramos

			mc.times.resize(keys); // lista de tiempos de cada clave
			for (size_t k = 0; k < keys; k++)
				ReadElement(g, sampler.input, k, &mc.times[k], 1); // leemos cada tiempo

			mc.values.resize(keys * perKey * comps); // lista de valores de todas las claves
			for (size_t k = 0; k < keys * perKey; k++)
				ReadElement(g, sampler.output, k, &mc.values[k * comps], (int)comps); // leemos cada valor

			out.duration = std::max(out.duration, mc.times.back()); // la animacion dura lo que dure su canal mas largo
			nodes[mc.node].animated = true; // marcamos el nodo como animado
			out.channels.push_back(mc); // guardamos el canal en la animacion
		}
		animations.push_back(out); // guardamos la animacion en el modelo
	}

	// materiales y mallas
	GLuint white = SolidTexture(255, 255, 255); // textura blanca de 1 pixel para materiales sin imagen
	textures.push_back(white); // la anotamos para borrarla al final
	std::map<int, GLuint> cache; // texturas ya subidas a la GPU (para no repetirlas)

	std::vector<glm::vec3> localMin, localMax; // limites de cada primitiva para centrar y escalar el modelo

	for (size_t ni = 0; ni < g.nodes.size(); ni++) // recorremos los nodos buscando los que tienen malla
	{
		const tinygltf::Node& node = g.nodes[ni];
		if (node.mesh < 0) continue; // este nodo no tiene malla

		const tinygltf::Mesh& mesh = g.meshes[node.mesh]; // la malla del nodo
		for (size_t pi = 0; pi < mesh.primitives.size(); pi++) // una malla se divide en primitivas (una por material)
		{
			const tinygltf::Primitive& p = mesh.primitives[pi];
			if (p.mode != TINYGLTF_MODE_TRIANGLES) continue; // solo usamos triangulos

			auto attr = [&](const char* name) -> int // busca el numero de accessor de un atributo por su nombre
				{
					auto it = p.attributes.find(name);
					return (it == p.attributes.end()) ? -1 : it->second; // -1 si el atributo no existe
				};
			int posA = attr("POSITION"); // posiciones de los vertices
			int norA = attr("NORMAL"); // normales
			int uvA = attr("TEXCOORD_0"); // coordenadas de textura
			int jntA = attr("JOINTS_0"); // huesos que afectan a cada vertice
			int wgtA = attr("WEIGHTS_0"); // cuanto afecta cada hueso
			if (posA < 0) continue; // sin posiciones no hay nada que dibujar

			bool skinned = (node.skin >= 0 && jntA >= 0 && wgtA >= 0); // true si la malla se mueve con huesos

			// vertices
			size_t vertexCount = (size_t)g.accessors[posA].count; // cantidad de vertices
			std::vector<ModelVertex> verts(vertexCount); // lista de vertices que subiremos a la GPU
			glm::vec3 bmin(FLT_MAX), bmax(-FLT_MAX); // esquinas de la caja que encierra la malla
			for (size_t v = 0; v < vertexCount; v++) // recorremos cada vertice
			{
				ModelVertex& mv = verts[v];
				std::memset(&mv, 0, sizeof(mv)); // empezamos con todo en cero
				float f[4] = { 0, 0, 0, 0 }; // espacio temporal para leer numeros

				ReadElement(g, posA, v, f, 3); // leemos la posicion (x, y, z)
				mv.position[0] = f[0]; mv.position[1] = f[1]; mv.position[2] = f[2];
				bmin = glm::min(bmin, glm::vec3(f[0], f[1], f[2])); // actualizamos la esquina minima de la caja
				bmax = glm::max(bmax, glm::vec3(f[0], f[1], f[2])); // y la maxima

				mv.normal[1] = 1.0f; // si no trae normales, apuntan hacia arriba
				if (norA >= 0) // si trae normales
				{
					ReadElement(g, norA, v, f, 3); // las leemos
					mv.normal[0] = f[0]; mv.normal[1] = f[1]; mv.normal[2] = f[2];
				}
				if (uvA >= 0) // si trae coordenadas de textura
				{
					ReadElement(g, uvA, v, f, 2); // las leemos (u, v)
					mv.texUV[0] = f[0]; mv.texUV[1] = f[1];
				}
				if (skinned) // si la malla tiene huesos
				{
					float j[4] = { 0, 0, 0, 0 }; // numeros de los 4 huesos del vertice
					ReadElement(g, jntA, v, j, 4);
					ReadElement(g, wgtA, v, f, 4); // peso de cada hueso
					float sum = f[0] + f[1] + f[2] + f[3]; // suma de pesos (deberia dar 1)
					for (int k = 0; k < 4; k++)
					{
						mv.joints[k] = (j[k] < MODEL_MAX_JOINTS) ? j[k] : 0.0f; // si el hueso no cabe en el shader, usamos el 0
						mv.weights[k] = (sum > 0.0f) ? f[k] / sum : (k == 0 ? 1.0f : 0.0f); // normalizamos los pesos para que sumen 1
					}
				}
			}

			// indices
			std::vector<GLuint> idx; // lista de indices: cada 3 forman un triangulo
			if (p.indices >= 0) // si el modelo trae indices
			{
				idx.resize((size_t)g.accessors[p.indices].count);
				for (size_t i = 0; i < idx.size(); i++)
				{
					float f = 0.0f;
					ReadElement(g, p.indices, i, &f, 1); // leemos cada indice
					idx[i] = (GLuint)f;
				}
			}
			else // si no trae, los vertices ya estan en orden: 0, 1, 2, 3...
			{
				idx.resize(vertexCount);
				for (size_t i = 0; i < idx.size(); i++) idx[i] = (GLuint)i;
			}

			// material
			ModelPrimitive prim; // la primitiva que vamos a guardar
			prim.baseColor = glm::vec4(1.0f); // color base (blanco = no cambia la textura)
			prim.alphaCutoff = 0.0f; // por defecto no se descarta ningun pixel
			prim.doubleSided = false; // por defecto solo se ve por un lado
			prim.texture = white; // por defecto la textura blanca
			if (p.material >= 0) // si la primitiva tiene material
			{
				const tinygltf::Material& mat = g.materials[p.material];

				int colorTex = mat.pbrMetallicRoughness.baseColorTexture.index; // numero de la textura de color
				const std::vector<double>& bc = mat.pbrMetallicRoughness.baseColorFactor; // color que multiplica la textura
				if (bc.size() == 4) prim.baseColor = glm::vec4((float)bc[0], (float)bc[1], (float)bc[2], (float)bc[3]);

				// algunos modelos viejos usan la extension "specular glossiness" en vez del material normal
				auto ext = mat.extensions.find("KHR_materials_pbrSpecularGlossiness");
				if (colorTex < 0 && ext != mat.extensions.end()) // si no hay textura normal pero si la de la extension
				{
					const tinygltf::Value& e = ext->second;
					if (e.Has("diffuseTexture")) colorTex = e.Get("diffuseTexture").Get("index").GetNumberAsInt(); // textura de color de la extension
					if (e.Has("diffuseFactor") && e.Get("diffuseFactor").ArrayLen() == 4) // color base de la extension
					{
						const tinygltf::Value& df = e.Get("diffuseFactor");
						prim.baseColor = glm::vec4((float)df.Get(0).GetNumberAsDouble(), (float)df.Get(1).GetNumberAsDouble(), (float)df.Get(2).GetNumberAsDouble(), (float)df.Get(3).GetNumberAsDouble());
					}
				}
				prim.texture = LoadGltfTexture(g, colorTex, cache, textures, white); // cargamos (o reutilizamos) la textura

				if (mat.alphaMode == "MASK") prim.alphaCutoff = (float)mat.alphaCutoff; // transparencia de recorte: se descartan los pixeles bajo ese alfa
				else if (mat.alphaMode == "BLEND") prim.alphaCutoff = 0.1f; // sin mezcla: se recortan los pixeles transparentes
				prim.doubleSided = mat.doubleSided; // si el material se ve por los dos lados
			}

			// subimos todo a la GPU
			prim.indexCount = (GLsizei)idx.size(); // cuantos indices hay que dibujar
			prim.node = (int)ni; // nodo al que pertenece
			prim.skin = skinned ? node.skin : -1; // esqueleto que la mueve (-1 si no tiene)

			glGenVertexArrays(1, &prim.vao); // VAO: guarda como leer los datos de los vertices
			glGenBuffers(1, &prim.vbo); // VBO: guarda los vertices
			glGenBuffers(1, &prim.ebo); // EBO: guarda los indices
			glBindVertexArray(prim.vao); // activamos el VAO
			glBindBuffer(GL_ARRAY_BUFFER, prim.vbo);
			glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(ModelVertex), verts.data(), GL_STATIC_DRAW); // subimos los vertices (no cambian, por eso STATIC)
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, prim.ebo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW); // subimos los indices
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ModelVertex), (void*)(0 * sizeof(float)));  // posicion
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ModelVertex), (void*)(3 * sizeof(float)));  // normal
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(ModelVertex), (void*)(6 * sizeof(float)));  // uv
			glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(ModelVertex), (void*)(8 * sizeof(float)));  // huesos
			glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(ModelVertex), (void*)(12 * sizeof(float))); // pesos
			for (int a = 0; a < 5; a++) glEnableVertexAttribArray(a); // activamos los 5 atributos
			glBindVertexArray(0); // desactivamos el VAO
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			primitives.push_back(prim); // guardamos la primitiva en el modelo
			localMin.push_back(bmin); // y los limites de su caja
			localMax.push_back(bmax);
		}
	}

	// centrar y escalar: pies en y = 0, centrado en x y z, con alto targetHeight ---
	Update(0.0f, -1); // pose de reposo

	glm::vec3 lo(FLT_MAX), hi(-FLT_MAX); // esquinas de la caja que encierra todo el modelo
	for (size_t i = 0; i < primitives.size(); i++) // recorremos todas las primitivas
	{
		// las mallas con esqueleto ya estan en el espacio del modelo; las demas pasan por la matriz de su nodo
		glm::mat4 m = (primitives[i].skin >= 0) ? glm::mat4(1.0f) : nodes[primitives[i].node].world;
		for (int c = 0; c < 8; c++) // las 8 esquinas de la caja de la primitiva
		{
			glm::vec3 corner((c & 1) ? localMax[i].x : localMin[i].x, (c & 2) ? localMax[i].y : localMin[i].y, (c & 4) ? localMax[i].z : localMin[i].z); // cada bit de c elige minimo o maximo en x, y, z
			glm::vec3 w = glm::vec3(m * glm::vec4(corner, 1.0f)); // la esquina ya colocada en el modelo
			lo = glm::min(lo, w); // actualizamos la caja total
			hi = glm::max(hi, w);
		}
	}

	float h = hi.y - lo.y; // alto del modelo original
	std::cout << "Altura original del modelo: " << h << std::endl;
	std::cout << "Altura solicitada: " << targetHeight << std::endl;
	if (primitives.empty() || h <= 0.0f) // si no hay nada que dibujar
	{
		std::cout << "El modelo no tiene geometria visible: " << file << std::endl;
		normMatrix = glm::mat4(1.0f); // no lo escalamos
	}
	else
	{
		float s = targetHeight / h; // cuanto hay que agrandar o achicar para llegar al alto pedido
		std::cout << "Escala calculada: " << s << std::endl;
		normMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(s)) * glm::translate(glm::mat4(1.0f), glm::vec3(-(lo.x + hi.x) * 0.5f, -lo.y, -(lo.z + hi.z) * 0.5f)); // primero lo centramos en x y z con los pies en y = 0, luego lo escalamos
	}

	std::cout << "Modelo cargado: " << file << " (" << primitives.size() << " mallas, " << animations.size() << " animaciones)" << std::endl;
	for (size_t a = 0; a < animations.size(); a++) // mostramos el nombre y la duracion de cada animacion
		std::cout << "   animacion " << a << ": " << animations[a].name << " (" << animations[a].duration << " s)" << std::endl;

	return true; // el modelo se cargo bien
}


// animacion


// calcula la matriz final de un nodo y, recursivamente, la de todos sus hijos
void Model::UpdateNode(int i, const glm::mat4& parentWorld, const std::vector<glm::mat4>& local)
{
	nodes[i].world = parentWorld * local[i]; // la matriz del nodo = la del padre * la suya
	for (size_t c = 0; c < nodes[i].children.size(); c++) // repetimos con cada hijo
		UpdateNode(nodes[i].children[c], nodes[i].world, local);
}

// calcula la pose del modelo en el segundo "time" de la animacion "animation" (-1 = pose de reposo)
void Model::Update(float time, int animation)
{
	size_t n = nodes.size(); // cantidad de nodos
	std::vector<glm::vec3> T(n), S(n); // posicion y escala actual de cada nodo
	std::vector<glm::quat> R(n); // rotacion actual de cada nodo
	for (size_t i = 0; i < n; i++) // empezamos con la pose de reposo
	{
		T[i] = nodes[i].translation;
		R[i] = nodes[i].rotation;
		S[i] = nodes[i].scale;
	}

	bool playing = (animation >= 0 && animation < (int)animations.size()); // true si pidieron una animacion que existe
	if (playing)
	{
		const ModelAnimation& a = animations[animation];
		float t = (a.duration > 0.0f) ? std::fmod(time, a.duration) : 0.0f; // la animacion se repite
		for (size_t c = 0; c < a.channels.size(); c++) // aplicamos cada canal de la animacion
		{
			int target = a.channels[c].node; // nodo que mueve este canal
			SampleChannel(a.channels[c], t, T[target], R[target], S[target]); // cambia posicion, rotacion o escala de ese nodo
		}
	}

	// matriz local de cada nodo
	std::vector<glm::mat4> local(n);
	for (size_t i = 0; i < n; i++)
	{
		if (playing && nodes[i].animated) // si hay animacion y este nodo se mueve
			local[i] = glm::translate(glm::mat4(1.0f), T[i]) * glm::mat4_cast(R[i]) * glm::scale(glm::mat4(1.0f), S[i]); // armamos la matriz con la pose actual
		else
			local[i] = nodes[i].restLocal; // si no, usamos la de reposo
	}

	// matriz del mundo de cada nodo, empezando por las raices
	for (size_t i = 0; i < n; i++)
		if (nodes[i].parent < 0) // las raices no tienen padre
			UpdateNode((int)i, glm::mat4(1.0f), local);

	// matrices de los huesos: posicion actual del hueso * su inversa de reposo
	for (size_t s = 0; s < skins.size(); s++)
		for (size_t j = 0; j < skins[s].joints.size(); j++)
			skins[s].jointMatrices[j] = nodes[skins[s].joints[j]].world * skins[s].inverseBind[j];
}


// dibujo


// dibuja el modelo; "matrix" lo coloca, gira y escala en el mundo
void Model::Draw(Shader& shader, glm::mat4 matrix)
{
	shader.Activate(); // usamos el shader de los modelos

	glm::mat4 model = matrix * normMatrix; // primero lo escalamos y centramos (normMatrix) y luego lo colocamos (matrix)
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model)); // mandamos la matriz al shader
	glUniform1i(glGetUniformLocation(shader.ID, "diffuse0"), 0); // el shader lee la textura de la unidad 0
	glActiveTexture(GL_TEXTURE0);

	GLboolean culling = glIsEnabled(GL_CULL_FACE); // guardamos si el descarte de caras traseras estaba activo

	for (size_t i = 0; i < primitives.size(); i++) // dibujamos cada primitiva
	{
		const ModelPrimitive& p = primitives[i];

		if (p.skin >= 0) // la malla se mueve con huesos
		{
			glUniform1i(glGetUniformLocation(shader.ID, "skinned"), 1); // le decimos al shader que use huesos
			const std::vector<glm::mat4>& jm = skins[p.skin].jointMatrices; // matrices actuales de los huesos
			glUniformMatrix4fv(glGetUniformLocation(shader.ID, "joints"), (GLsizei)jm.size(), GL_FALSE, glm::value_ptr(jm[0])); // se las mandamos todas
		}
		else // la malla es una pieza rigida
		{
			glUniform1i(glGetUniformLocation(shader.ID, "skinned"), 0); // sin huesos
			glUniformMatrix4fv(glGetUniformLocation(shader.ID, "nodeMatrix"), 1, GL_FALSE, glm::value_ptr(nodes[p.node].world)); // usa la matriz de su nodo
		}

		glUniform4f(glGetUniformLocation(shader.ID, "baseColor"), p.baseColor.r, p.baseColor.g, p.baseColor.b, p.baseColor.a); // color que multiplica la textura
		glUniform1f(glGetUniformLocation(shader.ID, "alphaCutoff"), p.alphaCutoff); // alfa minimo para no descartar el pixel

		if (p.doubleSided) glDisable(GL_CULL_FACE); // si se ve por los dos lados, dibujamos las caras traseras
		else if (culling) glEnable(GL_CULL_FACE); // si no, restauramos el descarte

		glBindTexture(GL_TEXTURE_2D, p.texture); // activamos la textura de esta primitiva
		glBindVertexArray(p.vao); // activamos sus vertices
		glDrawElements(GL_TRIANGLES, p.indexCount, GL_UNSIGNED_INT, 0); // dibujamos los triangulos
	}

	glBindVertexArray(0); // desactivamos el VAO
	if (culling) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE); // dejamos el descarte como estaba
}

void Model::Delete() // libera la memoria de OpenGL
{
	for (size_t i = 0; i < primitives.size(); i++) // borra VAO, VBO y EBO de cada primitiva
	{
		glDeleteVertexArrays(1, &primitives[i].vao); // borra el VAO
		glDeleteBuffers(1, &primitives[i].vbo); // borra el VBO
		glDeleteBuffers(1, &primitives[i].ebo); // borra el EBO
	}
	for (size_t i = 0; i < textures.size(); i++) // borra todas las texturas
		glDeleteTextures(1, &textures[i]);

	primitives.clear(); // vaciamos la lista de primitivas
	textures.clear(); // vaciamos la lista de texturas
}