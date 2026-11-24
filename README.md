
# Minecraft: Wither Storm - OpenGL

## Información del proyecto

- Estudiante: Miguel Santamaría
- Código estudiantil:6014333
- Asignatura: Computación Gráfica
- Lenguaje: C++
- API gráfica: OpenGL 3.3 Core
- Entorno de desarrollo: Visual Studio 2022

## Descripción

El proyecto consiste en la creación de un entorno tridimensional
interactivo inspirado en Minecraft, desarrollado mediante C++ y
OpenGL.

El escenario representa un mundo dividido entre una zona natural
y una zona corrupta por el Wither Storm, en el cual se encuentran
estructuras (Casas), árboles y diferentes personajes importados en formato
GLB.

El usuario puede desplazarse libremente por el escenario, observar
los personajes, seleccionar materiales y construir o destruir
bloques mediante el teclado y el ratón.

El proyecto integra diferentes conceptos de computación gráfica,
como transformaciones matriciales, iluminación de Phong,
texturización, curvas de Bézier, animación esquelética y
generación de geometría tridimensional.

## Objetivos

- Implementar un entorno tridimensional interactivo.
- Aplicar matrices de traslación, rotación y proyección.
- Implementar el modelo de iluminación de Phong.
- Importar modelos tridimensionales en formato glTF y GLB.
- Generar trayectorias mediante curvas de Bézier.
- Implementar la interacción mediante teclado y ratón.
- Aplicar técnicas de texturización y generación de mallas.

## Tecnologías y librerías


| Tecnología | Función dentro del proyecto |
|---|---|
| C++17 | Lenguaje principal de programación. |
| OpenGL 3.3 | Renderizado de gráficos tridimensionales. |
| GLFW | Gestión de ventanas y entradas del usuario. |
| GLAD | Carga de las funciones de OpenGL. |
| GLM | Operaciones con vectores, matrices y cuaterniones. |
| stb_image | Carga de imágenes y texturas. |
| TinyGLTF | Importación de modelos GLTF y GLB. |
| nlohmann JSON | Procesamiento de datos JSON de los modelos. |
| Visual Studio 2022 Community | Entorno de desarrollo y compilación. |
| Git y GitHub | Control de versiones y almacenamiento del proyecto. |

## Arquitectura del software

El proyecto utiliza una arquitectura modular orientada a objetos,
separando las responsabilidades de generación del mundo,
renderizado, cámara, texturas, shaders e importación de modelos.

### Componentes principales

| Archivo o clase | Responsabilidad |
|---|---|
| main.cpp | Inicialización, ciclo principal, interacción y animaciones |
| Camera | Movimiento, orientación y matrices de cámara |
| World | Generación del terreno, estructuras y gestión de bloques |
| Mesh | Creación, actualización y renderizado de mallas |
| Model | Importación y animación de modelos tridimensionales |
| Texture | Carga y vinculación de texturas |
| Shader | Compilación y vinculación de shaders |
| VAO | Configuración de los atributos de los vértices |
| VBO | Almacenamiento de vértices en la GPU |
| EBO | Almacenamiento de índices en la GPU |



## Implementación de computación gráfica

### 1. Generación del mundo

El terreno está compuesto por una matriz tridimensional de
64 x 16 x 64 bloques.

Cada bloque ocupa una unidad en los tres ejes y pertenece a uno
de los siguientes materiales:

- Aire
- Pasto
- Tierra
- Piedra
- Madera
- Hojas
- Tablas

Para optimizar el renderizado, únicamente se generan las caras
de los bloques que no están cubiertas por bloques vecinos.

También se generan dos casas, una parcela de construcción y
18 árboles distribuidos de manera pseudoaleatoria.

### 2. Transformaciones matriciales

Se utiliza la librería GLM para realizar operaciones matemáticas
sobre vectores y matrices.

Las principales transformaciones implementadas son:

- Traslación mediante glm::translate().
- Rotación mediante glm::rotate().
- Escalado mediante glm::scale().
- Matriz de vista mediante glm::lookAt().
- Proyección en perspectiva mediante glm::perspective().

La matriz de cámara se obtiene multiplicando la matriz de
proyección por la matriz de vista.

### 3. Iluminación de Phong

El terreno utiliza el modelo de iluminación de Phong,
implementado en el fragment shader default.frag.

Este modelo está compuesto por:

- Luz ambiental: iluminación base del escenario.
- Luz difusa: iluminación dependiente del ángulo de incidencia.
- Luz especular: reflejo de la luz hacia la cámara.

Se utiliza un mapa especular para controlar la intensidad de los
reflejos de las superficies.

Los modelos importados utilizan iluminación ambiental y difusa
mediante un shader independiente.

### 4. Texturización

Se utiliza un atlas de texturas que contiene ocho superficies
distribuidas en cuatro columnas y dos filas.

Las coordenadas UV permiten seleccionar la textura correspondiente
a cada tipo de bloque y a cada una de sus caras.

El proyecto utiliza dos atlas:

- atlas.png: mapa de color.
- atlas_SPEC.png: mapa especular.

### 5. Importación de modelos tridimensionales

Mediante TinyGLTF se realiza la carga de modelos en formato
glTF y GLB.

La clase Model permite procesar:

- Geometría.
- Vértices e índices.
- Normales.
- Coordenadas de textura.
- Materiales.
- Transformaciones de nodos.
- Esqueletos y animaciones.

Los modelos son escalados para adaptarse a las dimensiones
del mundo.

Entre los personajes implementados se encuentran aldeanos,
abejas, zorros, gatos, pollos, allays, creepers, zombies,
magmas, wardens, Happy Ghast y Wither Storm.

### 6. Curvas de Bézier

Se utilizan curvas de Bézier cúbicas para generar trayectorias
de movimiento mediante cuatro puntos de control.

La ecuación utilizada es:

B(t) = (1-t)^3 P0 + 3(1-t)^2 t P1
       + 3(1-t)t^2 P2 + t^3 P3

Donde t pertenece al intervalo [0,1].

También se calcula la derivada de la curva para obtener el
vector tangente y orientar determinados personajes en la
dirección de su movimiento.

Las curvas se aplican a zombies, creepers, zorros, allays
y Happy Ghast.

En los creepers se implementa una variación de velocidad
mediante la función:

t = t²(3 - 2t)

Esto permite suavizar la aceleración y desaceleración durante
el recorrido.

### 7. Interacción y algoritmo DDA

Para construir y destruir bloques se utiliza un sistema de
Raycasting basado en el algoritmo DDA tridimensional.

Se genera un rayo desde la posición de la cámara en la
dirección hacia la que está mirando el usuario.

El algoritmo recorre las celdas del mundo hasta encontrar
un bloque dentro del alcance establecido.

Cuando se modifica un bloque, se actualizan los vértices
e índices de la malla del terreno.

### 8. Corrupción del escenario


El escenario cuenta con una zona de corrupción estática
inspirada en el Wither Storm, implementada directamente
en el fragment shader (default.frag).

La frontera de la corrupción se genera mediante la
combinación de funciones sinusoidales que dependen de
la posición de los fragmentos en el espacio 3D.

Se utiliza la función smoothstep() para generar una
transición suave entre el terreno normal y el corrupto,
mientras que la función mix() permite combinar los
colores originales con tonalidades oscuras y moradas.

La corrupción permanece fija durante toda la ejecución
del programa y no modifica la geometría, las texturas
originales ni los datos almacenados en la clase World.

## Controles

| Entrada | Acción |
|---|---|
| W | Avanzar |
| A | Desplazarse a la izquierda |
| S | Retroceder |
| D | Desplazarse a la derecha |
| Espacio | Ascender |
| Ctrl izquierdo | Descender |
| Shift izquierdo | Aumentar velocidad |
| Ratón | Rotar la cámara |
| Clic izquierdo | Destruir bloque |
| Clic derecho | Colocar bloque |
| 1 | Seleccionar pasto |
| 2 | Seleccionar tierra |
| 3 | Seleccionar piedra |
| 4 | Seleccionar madera |
| 5 | Seleccionar hojas |
| 6 | Seleccionar tablas |
| ESC | Cerrar el programa |

## Estructura principal de archivos
```
TutorialYt/
│
├── main.cpp
│
├── Camera.h
├── Camera.cpp
├── World.h
├── World.cpp
├── Mesh.h
├── Mesh.cpp
├── Model.h
├── Model.cpp
├── Texture.h
├── Texture.cpp
├── shaderClass.h
├── shaderClass.cpp
├── VAO.h
├── VAO.cpp
├── VBO.h
├── VBO.cpp
├── EBO.h
├── EBO.cpp
│
├── tiny_gltf.h
├── json.hpp
├── stb_image.h
├── stb.cpp
├── glad.c
│
├── default.vert
├── default.frag
├── light.vert
├── light.frag
├── model.vert
├── model.frag
│
├── atlas.png
├── atlas_SPEC.png
│
├── Models/
│   ├── Villager.glb
│   ├── Bee.glb
│   ├── Fox.glb
│   ├── Happy.glb
│   ├── Poulet.glb
│   ├── Cat.glb
│   ├── Allay.glb
│   ├── Creeper.glb
│   ├── Zombie.glb
│   ├── Magma.glb
│   ├── Warden.glb
│   └── Wither.glb
│
└── README.md
```
## Ejecución del proyecto

## Ejecución del proyecto

1. Clonar o descargar el repositorio.
2. Abrir la solución del proyecto en Visual Studio 2022.
3. Verificar la configuración de las librerías y dependencias.
4. Configurar el directorio de trabajo como $(ProjectDir)
   desde las propiedades de depuración del proyecto.
5. Comprobar que los shaders, texturas y modelos se encuentran
   en las rutas correspondientes.
6. Compilar y ejecutar el proyecto.

El programa utiliza rutas relativas para cargar los shaders,
los atlas de texturas y los modelos tridimensionales.

Por esta razón, es necesario ejecutar el programa desde
el directorio correspondiente al proyecto.

El programa se ejecuta en pantalla completa utilizando la
resolución del monitor.

## Video explicativo

/

## Conclusiones

El desarrollo del proyecto permitió integrar diferentes
conceptos matemáticos y computacionales en un entorno
tridimensional interactivo.

Mediante OpenGL fue posible implementar transformaciones
matriciales, iluminación, texturización, importación de
modelos, animaciones y trayectorias mediante curvas de Bézier.

La separación del código en diferentes clases facilita
el mantenimiento y la ampliación de las funcionalidades
del escenario.
