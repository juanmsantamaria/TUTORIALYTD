#include<stb/stb_image.h> // solo las declaraciones: la implementacion de stb_image ya esta en tu proyecto (la usa Texture.cpp)

// tinygltf (con su json.hpp) lee el archivo .gltf/.glb. Le decimos que no incluya su propia copia de stb_image
#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_INCLUDE_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include"tiny_gltf.h"

#include"Model.h"
#include<glm/gtc/type_ptr.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<iostream>
#include<algorithm>
#include<cmath>
#include<cfloat>
#include<cstring>
#include<cstdint>
#include<map>


// funciones de ayuda


// crea una textura de OpenGL de 1 pixel (se usa cuando el material no tiene imagen)
static GLuint SolidTexture(unsigned char r, unsigned char g, unsigned char b)
{
	unsigned char pixel[4] = { r, g, b, 255 };
	GLuint id;
	glGenTextures(1, &id);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
	return id;
}

// lee un numero de un accessor segun su tipo (float, byte, short...), normalizandolo si hace falta
static float ReadComponent(const unsigned char* p, int type, bool normalized)
{
	switch (type)
	{
	case TINYGLTF_COMPONENT_TYPE_FLOAT:
	{
		float v; std::memcpy(&v, p, 4); return v;
	}
	case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
	{
		float v = (float)(*p); return normalized ? v / 255.0f : v;
	}
	case TINYGLTF_COMPONENT_TYPE_BYTE:
	{
		float v = (float)(*(const int8_t*)p); return normalized ? std::max(v / 127.0f, -1.0f) : v;
	}
	case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
	{
		uint16_t v; std::memcpy(&v, p, 2); return normalized ? v / 65535.0f : (float)v;
	}
	case TINYGLTF_COMPONENT_TYPE_SHORT:
	{
		int16_t v; std::memcpy(&v, p, 2); return normalized ? std::max(v / 32767.0f, -1.0f) : (float)v;
	}
	case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
	{
		uint32_t v; std::memcpy(&v, p, 4); return (float)v;
	}
	}
	return 0.0f;
}

// lee el elemento "index" de un accessor (por ejemplo, un vertice) y lo guarda en out (maximo n numeros)
static void ReadElement(const tinygltf::Model& g, int accessorIndex, size_t index, float* out, int n)
{
	for (int c = 0; c < n; c++) out[c] = 0.0f;
	if (accessorIndex < 0) return;

	const tinygltf::Accessor& a = g.accessors[accessorIndex];
	if (a.bufferView < 0) return; // sin datos: todo ceros

	const tinygltf::BufferView& bv = g.bufferViews[a.bufferView];
	const tinygltf::Buffer& buf = g.buffers[bv.buffer];

	int comps = tinygltf::GetNumComponentsInType(a.type);
	int compSize = tinygltf::GetComponentSizeInBytes(a.componentType);
	size_t stride = (size_t)a.ByteStride(bv);
	if (stride == 0) stride = (size_t)(comps * compSize);

	const unsigned char* p = buf.data.data() + bv.byteOffset + a.byteOffset + index * stride;
	for (int c = 0; c < comps && c < n; c++)
		out[c] = ReadComponent(p + c * compSize, a.componentType, a.normalized);
}

// carga una textura del glTF a OpenGL (tinygltf ya decodifico la imagen)
static GLuint LoadGltfTexture(const tinygltf::Model& g, int textureIndex, std::map<int, GLuint>& cache, std::vector<GLuint>& created, GLuint white)
{
	if (textureIndex < 0 || textureIndex >= (int)g.textures.size()) return white;

	auto found = cache.find(textureIndex);
	if (found != cache.end()) return found->second;

	const tinygltf::Texture& tex = g.textures[textureIndex];
	GLuint id;

	if (tex.source < 0 || tex.source >= (int)g.images.size() || g.images[tex.source].image.empty() || g.images[tex.source].bits != 8)
	{
		std::cout << "No se pudo cargar una textura del modelo (" << (tex.source >= 0 && tex.source < (int)g.images.size() ? g.images[tex.source].uri : std::string("sin imagen")) << ")" << std::endl;
		id = SolidTexture(255, 0, 255); // magenta = falta la textura
		created.push_back(id);
		cache[textureIndex] = id;
		return id;
	}

	const tinygltf::Image& img = g.images[tex.source];

	// pasamos la imagen a RGBA sin importar cuantos canales traiga
	std::vector<unsigned char> rgba((size_t)img.width * img.height * 4);
	for (size_t i = 0; i < (size_t)img.width * img.height; i++)
	{
		const unsigned char* s = &img.image[i * img.component];
		unsigned char* d = &rgba[i * 4];
		if (img.component == 1)      { d[0] = d[1] = d[2] = s[0]; d[3] = 255; }
		else if (img.component == 2) { d[0] = d[1] = d[2] = s[0]; d[3] = s[1]; }
		else if (img.component == 3) { d[0] = s[0]; d[1] = s[1]; d[2] = s[2]; d[3] = 255; }
		else                         { d[0] = s[0]; d[1] = s[1]; d[2] = s[2]; d[3] = s[3]; }
	}

	GLint wrapS = GL_REPEAT, wrapT = GL_REPEAT;
	if (tex.sampler >= 0 && tex.sampler < (int)g.samplers.size())
	{
		wrapS = g.samplers[tex.sampler].wrapS;
		wrapT = g.samplers[tex.sampler].wrapT;
	}

	glGenTextures(1, &id);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // pixeles cuadrados, como en Minecraft
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width, img.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba.data());
	glGenerateMipmap(GL_TEXTURE_2D);

	created.push_back(id);
	cache[textureIndex] = id;
	return id;
}

// valor de un canal de animacion en el instante t
static void SampleChannel(const ModelChannel& c, float t, glm::vec3& position, glm::quat& rotation, glm::vec3& scale)
{
	size_t n = c.times.size();
	if (n == 0) return;

	size_t comps = (c.path == 1) ? 4 : 3;
	size_t stride = (c.interp == 2) ? 3 : 1; // la cubica guarda 3 valores por clave: tangente entrada, valor y tangente salida
	size_t offset = (c.interp == 2) ? 1 : 0;

	// buscamos entre que dos claves esta t
	size_t k0, k1;
	float f = 0.0f;
	if (t <= c.times.front()) { k0 = k1 = 0; }
	else if (t >= c.times.back()) { k0 = k1 = n - 1; }
	else
	{
		k1 = 1;
		while (c.times[k1] < t) k1++;
		k0 = k1 - 1;
		f = (t - c.times[k0]) / (c.times[k1] - c.times[k0]);
		if (c.interp == 1) f = 0.0f; // escalon: se queda con la clave anterior
	}

	float a[4] = { 0, 0, 0, 1 }, b[4] = { 0, 0, 0, 1 };
	for (size_t j = 0; j < comps; j++)
	{
		a[j] = c.values[(k0 * stride + offset) * comps + j];
		b[j] = c.values[(k1 * stride + offset) * comps + j];
	}

	if (c.path == 0)
		position = glm::mix(glm::vec3(a[0], a[1], a[2]), glm::vec3(b[0], b[1], b[2]), f);
	else if (c.path == 2)
		scale = glm::mix(glm::vec3(a[0], a[1], a[2]), glm::vec3(b[0], b[1], b[2]), f);
	else
	{
		glm::quat qa(a[3], a[0], a[1], a[2]); // glTF guarda x, y, z, w; glm::quat recibe w, x, y, z
		glm::quat qb(b[3], b[0], b[1], b[2]);
		rotation = glm::normalize(glm::slerp(qa, qb, f));
	}
}



// carga del modelo

bool Model::Load(const char* file, float targetHeight)
{
	tinygltf::TinyGLTF loader;
	tinygltf::Model g;
	std::string err, warn;

	std::string path = file;
	bool binary = path.size() >= 4 && (path.substr(path.size() - 4) == ".glb" || path.substr(path.size() - 4) == ".GLB");

	stbi_set_flip_vertically_on_load(false); // en glTF la imagen ya viene con el origen arriba a la izquierda
	bool ok = binary ? loader.LoadBinaryFromFile(&g, &err, &warn, path) : loader.LoadASCIIFromFile(&g, &err, &warn, path);
	stbi_set_flip_vertically_on_load(true); // dejamos la opcion como la usa la clase Texture

	if (!warn.empty()) std::cout << "Aviso al leer el modelo: " << warn << std::endl;
	if (!ok)
	{
		std::cout << "No se pudo leer el modelo: " << file << " " << err << std::endl;
		return false;
	}

	//  nodos 
	nodes.resize(g.nodes.size());
	for (size_t i = 0; i < g.nodes.size(); i++)
	{
		const tinygltf::Node& n = g.nodes[i];
		ModelNode& m = nodes[i];

		m.parent = -1; // se completa abajo con los hijos de cada nodo
		m.children = n.children;
		m.translation = glm::vec3(0.0f);
		m.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		m.scale = glm::vec3(1.0f);
		if (n.translation.size() == 3) m.translation = glm::vec3((float)n.translation[0], (float)n.translation[1], (float)n.translation[2]);
		if (n.rotation.size() == 4) m.rotation = glm::quat((float)n.rotation[3], (float)n.rotation[0], (float)n.rotation[1], (float)n.rotation[2]); // glTF: x, y, z, w
		if (n.scale.size() == 3) m.scale = glm::vec3((float)n.scale[0], (float)n.scale[1], (float)n.scale[2]);

		if (n.matrix.size() == 16)
		{
			float mm[16];
			for (int k = 0; k < 16; k++) mm[k] = (float)n.matrix[k];
			m.restLocal = glm::make_mat4(mm);
		}
		else
		{
			m.restLocal = glm::translate(glm::mat4(1.0f), m.translation) * glm::mat4_cast(m.rotation) * glm::scale(glm::mat4(1.0f), m.scale);
		}
		m.animated = false;
		m.world = glm::mat4(1.0f);
	}
	for (size_t i = 0; i < nodes.size(); i++)
		for (size_t c = 0; c < nodes[i].children.size(); c++)
			nodes[nodes[i].children[c]].parent = (int)i;

	// esqueletos 
	skins.resize(g.skins.size());
	for (size_t s = 0; s < g.skins.size(); s++)
	{
		const tinygltf::Skin& skin = g.skins[s];
		ModelSkin& sk = skins[s];

		size_t jointCount = skin.joints.size();
		if (jointCount > MODEL_MAX_JOINTS)
		{
			std::cout << "Aviso: el modelo tiene " << jointCount << " huesos y solo se usan " << MODEL_MAX_JOINTS << " (sube MODEL_MAX_JOINTS en Model.h y MAX_JOINTS en model.vert)" << std::endl;
			jointCount = MODEL_MAX_JOINTS;
		}

		sk.joints.assign(skin.joints.begin(), skin.joints.begin() + jointCount);
		sk.inverseBind.assign(jointCount, glm::mat4(1.0f));
		if (skin.inverseBindMatrices >= 0)
		{
			for (size_t j = 0; j < jointCount; j++)
			{
				float raw[16];
				ReadElement(g, skin.inverseBindMatrices, j, raw, 16);
				sk.inverseBind[j] = glm::make_mat4(raw);
			}
		}
		sk.jointMatrices.assign(jointCount, glm::mat4(1.0f));
	}

	//  animaciones 
	for (size_t a = 0; a < g.animations.size(); a++)
	{
		const tinygltf::Animation& anim = g.animations[a];
		ModelAnimation out;
		out.name = anim.name.empty() ? ("animacion " + std::to_string(a)) : anim.name;
		out.duration = 0.0f;

		for (size_t c = 0; c < anim.channels.size(); c++)
		{
			const tinygltf::AnimationChannel& ch = anim.channels[c];
			if (ch.target_node < 0 || ch.sampler < 0 || ch.sampler >= (int)anim.samplers.size()) continue;
			const tinygltf::AnimationSampler& sampler = anim.samplers[ch.sampler];

			ModelChannel mc;
			mc.node = ch.target_node;
			if (ch.target_path == "translation") mc.path = 0;
			else if (ch.target_path == "rotation") mc.path = 1;
			else if (ch.target_path == "scale") mc.path = 2;
			else continue; // (animaciones de "morph targets" no se usan)

			if (sampler.interpolation == "STEP") mc.interp = 1;
			else if (sampler.interpolation == "CUBICSPLINE") mc.interp = 2;
			else mc.interp = 0;

			size_t keys = (size_t)g.accessors[sampler.input].count;
			size_t comps = (mc.path == 1) ? 4 : 3;
			size_t perKey = (mc.interp == 2) ? 3 : 1;
			if (keys == 0 || (size_t)g.accessors[sampler.output].count < keys * perKey) continue; // canal danado, lo ignoramos

			mc.times.resize(keys);
			for (size_t k = 0; k < keys; k++)
				ReadElement(g, sampler.input, k, &mc.times[k], 1);

			mc.values.resize(keys * perKey * comps);
			for (size_t k = 0; k < keys * perKey; k++)
				ReadElement(g, sampler.output, k, &mc.values[k * comps], (int)comps);

			out.duration = std::max(out.duration, mc.times.back());
			nodes[mc.node].animated = true;
			out.channels.push_back(mc);
		}
		animations.push_back(out);
	}

	// materiales y mallas
	GLuint white = SolidTexture(255, 255, 255);
	textures.push_back(white);
	std::map<int, GLuint> cache;

	std::vector<glm::vec3> localMin, localMax; // limites de cada primitiva para centrar y escalar el modelo

	for (size_t ni = 0; ni < g.nodes.size(); ni++)
	{
		const tinygltf::Node& node = g.nodes[ni];
		if (node.mesh < 0) continue;

		const tinygltf::Mesh& mesh = g.meshes[node.mesh];
		for (size_t pi = 0; pi < mesh.primitives.size(); pi++)
		{
			const tinygltf::Primitive& p = mesh.primitives[pi];
			if (p.mode != TINYGLTF_MODE_TRIANGLES) continue;

			auto attr = [&](const char* name) -> int
			{
				auto it = p.attributes.find(name);
				return (it == p.attributes.end()) ? -1 : it->second;
			};
			int posA = attr("POSITION");
			int norA = attr("NORMAL");
			int uvA = attr("TEXCOORD_0");
			int jntA = attr("JOINTS_0");
			int wgtA = attr("WEIGHTS_0");
			if (posA < 0) continue;

			bool skinned = (node.skin >= 0 && jntA >= 0 && wgtA >= 0);

			// vertices
			size_t vertexCount = (size_t)g.accessors[posA].count;
			std::vector<ModelVertex> verts(vertexCount);
			glm::vec3 bmin(FLT_MAX), bmax(-FLT_MAX);
			for (size_t v = 0; v < vertexCount; v++)
			{
				ModelVertex& mv = verts[v];
				std::memset(&mv, 0, sizeof(mv));
				float f[4] = { 0, 0, 0, 0 };

				ReadElement(g, posA, v, f, 3);
				mv.position[0] = f[0]; mv.position[1] = f[1]; mv.position[2] = f[2];
				bmin = glm::min(bmin, glm::vec3(f[0], f[1], f[2]));
				bmax = glm::max(bmax, glm::vec3(f[0], f[1], f[2]));

				mv.normal[1] = 1.0f; // si no trae normales, apuntan hacia arriba
				if (norA >= 0)
				{
					ReadElement(g, norA, v, f, 3);
					mv.normal[0] = f[0]; mv.normal[1] = f[1]; mv.normal[2] = f[2];
				}
				if (uvA >= 0)
				{
					ReadElement(g, uvA, v, f, 2);
					mv.texUV[0] = f[0]; mv.texUV[1] = f[1];
				}
				if (skinned)
				{
					float j[4] = { 0, 0, 0, 0 };
					ReadElement(g, jntA, v, j, 4);
					ReadElement(g, wgtA, v, f, 4);
					float sum = f[0] + f[1] + f[2] + f[3];
					for (int k = 0; k < 4; k++)
					{
						mv.joints[k] = (j[k] < MODEL_MAX_JOINTS) ? j[k] : 0.0f;
						mv.weights[k] = (sum > 0.0f) ? f[k] / sum : (k == 0 ? 1.0f : 0.0f);
					}
				}
			}

			// indices
			std::vector<GLuint> idx;
			if (p.indices >= 0)
			{
				idx.resize((size_t)g.accessors[p.indices].count);
				for (size_t i = 0; i < idx.size(); i++)
				{
					float f = 0.0f;
					ReadElement(g, p.indices, i, &f, 1);
					idx[i] = (GLuint)f;
				}
			}
			else
			{
				idx.resize(vertexCount);
				for (size_t i = 0; i < idx.size(); i++) idx[i] = (GLuint)i;
			}

			// material
			ModelPrimitive prim;
			prim.baseColor = glm::vec4(1.0f);
			prim.alphaCutoff = 0.0f;
			prim.doubleSided = false;
			prim.texture = white;
			if (p.material >= 0)
			{
				const tinygltf::Material& mat = g.materials[p.material];

				int colorTex = mat.pbrMetallicRoughness.baseColorTexture.index;
				const std::vector<double>& bc = mat.pbrMetallicRoughness.baseColorFactor;
				if (bc.size() == 4) prim.baseColor = glm::vec4((float)bc[0], (float)bc[1], (float)bc[2], (float)bc[3]);

				// algunos modelos viejos usan la extension "specular glossiness" en vez del material normal
				auto ext = mat.extensions.find("KHR_materials_pbrSpecularGlossiness");
				if (colorTex < 0 && ext != mat.extensions.end())
				{
					const tinygltf::Value& e = ext->second;
					if (e.Has("diffuseTexture")) colorTex = e.Get("diffuseTexture").Get("index").GetNumberAsInt();
					if (e.Has("diffuseFactor") && e.Get("diffuseFactor").ArrayLen() == 4)
					{
						const tinygltf::Value& df = e.Get("diffuseFactor");
						prim.baseColor = glm::vec4((float)df.Get(0).GetNumberAsDouble(), (float)df.Get(1).GetNumberAsDouble(), (float)df.Get(2).GetNumberAsDouble(), (float)df.Get(3).GetNumberAsDouble());
					}
				}
				prim.texture = LoadGltfTexture(g, colorTex, cache, textures, white);

				if (mat.alphaMode == "MASK") prim.alphaCutoff = (float)mat.alphaCutoff;
				else if (mat.alphaMode == "BLEND") prim.alphaCutoff = 0.1f; // sin mezcla: se recortan los pixeles transparentes
				prim.doubleSided = mat.doubleSided;
			}

			// subimos todo a la GPU
			prim.indexCount = (GLsizei)idx.size();
			prim.node = (int)ni;
			prim.skin = skinned ? node.skin : -1;

			glGenVertexArrays(1, &prim.vao);
			glGenBuffers(1, &prim.vbo);
			glGenBuffers(1, &prim.ebo);
			glBindVertexArray(prim.vao);
			glBindBuffer(GL_ARRAY_BUFFER, prim.vbo);
			glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(ModelVertex), verts.data(), GL_STATIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, prim.ebo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ModelVertex), (void*)(0 * sizeof(float)));  // posicion
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ModelVertex), (void*)(3 * sizeof(float)));  // normal
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(ModelVertex), (void*)(6 * sizeof(float)));  // uv
			glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(ModelVertex), (void*)(8 * sizeof(float)));  // huesos
			glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(ModelVertex), (void*)(12 * sizeof(float))); // pesos
			for (int a = 0; a < 5; a++) glEnableVertexAttribArray(a);
			glBindVertexArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			primitives.push_back(prim);
			localMin.push_back(bmin);
			localMax.push_back(bmax);
		}
	}

	// centrar y escalar: pies en y = 0, centrado en x y z, con alto targetHeight ---
	Update(0.0f, -1); // pose de reposo

	glm::vec3 lo(FLT_MAX), hi(-FLT_MAX);
	for (size_t i = 0; i < primitives.size(); i++)
	{
		// las mallas con esqueleto ya estan en el espacio del modelo; las demas pasan por la matriz de su nodo
		glm::mat4 m = (primitives[i].skin >= 0) ? glm::mat4(1.0f) : nodes[primitives[i].node].world;
		for (int c = 0; c < 8; c++)
		{
			glm::vec3 corner((c & 1) ? localMax[i].x : localMin[i].x, (c & 2) ? localMax[i].y : localMin[i].y, (c & 4) ? localMax[i].z : localMin[i].z);
			glm::vec3 w = glm::vec3(m * glm::vec4(corner, 1.0f));
			lo = glm::min(lo, w);
			hi = glm::max(hi, w);
		}
	}

	float h = hi.y - lo.y;
	std::cout << "Altura original del modelo: " << h << std::endl;
	std::cout << "Altura solicitada: " << targetHeight << std::endl;
	if (primitives.empty() || h <= 0.0f)
	{
		std::cout << "El modelo no tiene geometria visible: " << file << std::endl;
		normMatrix = glm::mat4(1.0f);
	}
	else
	{
		float s = targetHeight / h;
		std::cout << "Escala calculada: " << s << std::endl;
		normMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(s)) * glm::translate(glm::mat4(1.0f), glm::vec3(-(lo.x + hi.x) * 0.5f, -lo.y, -(lo.z + hi.z) * 0.5f));
	}

	std::cout << "Modelo cargado: " << file << " (" << primitives.size() << " mallas, " << animations.size() << " animaciones)" << std::endl;
	for (size_t a = 0; a < animations.size(); a++)
		std::cout << "   animacion " << a << ": " << animations[a].name << " (" << animations[a].duration << " s)" << std::endl;

	return true;
}


// animacion


void Model::UpdateNode(int i, const glm::mat4& parentWorld, const std::vector<glm::mat4>& local)
{
	nodes[i].world = parentWorld * local[i];
	for (size_t c = 0; c < nodes[i].children.size(); c++)
		UpdateNode(nodes[i].children[c], nodes[i].world, local);
}

void Model::Update(float time, int animation)
{
	size_t n = nodes.size();
	std::vector<glm::vec3> T(n), S(n);
	std::vector<glm::quat> R(n);
	for (size_t i = 0; i < n; i++)
	{
		T[i] = nodes[i].translation;
		R[i] = nodes[i].rotation;
		S[i] = nodes[i].scale;
	}

	bool playing = (animation >= 0 && animation < (int)animations.size());
	if (playing)
	{
		const ModelAnimation& a = animations[animation];
		float t = (a.duration > 0.0f) ? std::fmod(time, a.duration) : 0.0f; // la animacion se repite
		for (size_t c = 0; c < a.channels.size(); c++)
		{
			int target = a.channels[c].node;
			SampleChannel(a.channels[c], t, T[target], R[target], S[target]);
		}
	}

	// matriz local de cada nodo
	std::vector<glm::mat4> local(n);
	for (size_t i = 0; i < n; i++)
	{
		if (playing && nodes[i].animated)
			local[i] = glm::translate(glm::mat4(1.0f), T[i]) * glm::mat4_cast(R[i]) * glm::scale(glm::mat4(1.0f), S[i]);
		else
			local[i] = nodes[i].restLocal;
	}

	// matriz del mundo de cada nodo, empezando por las raices
	for (size_t i = 0; i < n; i++)
		if (nodes[i].parent < 0)
			UpdateNode((int)i, glm::mat4(1.0f), local);

	// matrices de los huesos: posicion actual del hueso * su inversa de reposo
	for (size_t s = 0; s < skins.size(); s++)
		for (size_t j = 0; j < skins[s].joints.size(); j++)
			skins[s].jointMatrices[j] = nodes[skins[s].joints[j]].world * skins[s].inverseBind[j];
}


// dibujo


void Model::Draw(Shader& shader, glm::mat4 matrix)
{
	shader.Activate();

	glm::mat4 model = matrix * normMatrix;
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniform1i(glGetUniformLocation(shader.ID, "diffuse0"), 0);
	glActiveTexture(GL_TEXTURE0);

	GLboolean culling = glIsEnabled(GL_CULL_FACE);

	for (size_t i = 0; i < primitives.size(); i++)
	{
		const ModelPrimitive& p = primitives[i];

		if (p.skin >= 0)
		{
			glUniform1i(glGetUniformLocation(shader.ID, "skinned"), 1);
			const std::vector<glm::mat4>& jm = skins[p.skin].jointMatrices;
			glUniformMatrix4fv(glGetUniformLocation(shader.ID, "joints"), (GLsizei)jm.size(), GL_FALSE, glm::value_ptr(jm[0]));
		}
		else
		{
			glUniform1i(glGetUniformLocation(shader.ID, "skinned"), 0);
			glUniformMatrix4fv(glGetUniformLocation(shader.ID, "nodeMatrix"), 1, GL_FALSE, glm::value_ptr(nodes[p.node].world));
		}

		glUniform4f(glGetUniformLocation(shader.ID, "baseColor"), p.baseColor.r, p.baseColor.g, p.baseColor.b, p.baseColor.a);
		glUniform1f(glGetUniformLocation(shader.ID, "alphaCutoff"), p.alphaCutoff);

		if (p.doubleSided) glDisable(GL_CULL_FACE);
		else if (culling) glEnable(GL_CULL_FACE);

		glBindTexture(GL_TEXTURE_2D, p.texture);
		glBindVertexArray(p.vao);
		glDrawElements(GL_TRIANGLES, p.indexCount, GL_UNSIGNED_INT, 0);
	}

	glBindVertexArray(0);
	if (culling) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
}

void Model::Delete()
{
	for (size_t i = 0; i < primitives.size(); i++)
	{
		glDeleteVertexArrays(1, &primitives[i].vao);
		glDeleteBuffers(1, &primitives[i].vbo);
		glDeleteBuffers(1, &primitives[i].ebo);
	}
	for (size_t i = 0; i < textures.size(); i++)
		glDeleteTextures(1, &textures[i]);

	primitives.clear();
	textures.clear();
}
