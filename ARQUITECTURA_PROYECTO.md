# Arquitectura del proyecto PUJ Ogre

## 1. Resumen para la sustentación

El proyecto es una aplicación de escritorio en C++23 que presenta un laberinto
en primera persona con Ogre3D. La aplicación `Laberinto` coordina la escena,
los controles, la lógica del enemigo y el flujo de juego. Ogre se encarga de
la representación y la escena; Bullet, mediante OgreBullet, simula los cuerpos
físicos; y VTK se usa en las utilidades de geometría, las superficies
paramétricas y la lectura de mallas.

La idea central de diseño es separar la **orquestación** (`Application`), la
**simulación y reglas** (`Maze`, `Player`, `Enemy`, `Throwable`, `Game`), y la
**presentación** (escena Ogre, `EnemyPresentation`, `ThrowablePresentation`,
HUD y pantalla de derrota). Los parámetros de escena y varios valores de
jugabilidad se declaran en archivos de recursos, en lugar de estar todos
codificados en las clases.

### Explicación breve

> “`Application` conecta los componentes y actualiza el juego cada frame.
> `Maze` convierte una cuadrícula de texto en geometría y ofrece consultas de
> navegación y colisión. `Player`, `Enemy` y `Throwable` mantienen la lógica y
> la simulación; sus clases de presentación actualizan los nodos de Ogre. El
> formato `.scenepp` describe la escena y sus parámetros, y los plugins
> permiten a Ogre cargar ese formato y mallas compatibles con VTK.”

## 2. Mapa de componentes

```text
main
 └── Application (OgreBites: ciclo de vida, entrada y coordinación)
     ├── Maze ─────────────── maze.txt → celdas, paredes, rutas y colisiones
     ├── Model ────────────── OgreBullet/Bullet → mundo y cuerpos físicos
     ├── Player ───────────── cuerpo del jugador, cámara e input
     ├── Enemy ────────────── estados y decisiones de navegación
     │    └── EnemyPresentation → nodo, movimiento visual y bounds
     ├── Throwable ────────── proyectil, vida útil y cooldown
     │    └── ThrowablePresentation → posición y visibilidad
     ├── Game ─────────────── estado PLAYING/LOST y contacto con el enemigo
     └── CooldownHud / LoseScreen → interfaz 2D

     fps.scenepp ── DotPPSceneLoader ── Ogre SceneManager
     materials.material ───────────── materiales Ogre

pujOgre (biblioteca)
 ├── FPSCamera
 ├── vtkParametricExpression
 └── vtkPolyData2OgreMesh
       ↑
       ├── Plugin_pujOgreDotPPSceneCodec
       └── Plugin_pujOgreVTKCodec
```

`FPSCamera` es una utilidad disponible en la biblioteca, pero la aplicación
actual `Laberinto` implementa su movimiento de primera persona mediante
`Player`; no instancia `pujOgre::FPSCamera`.

## 3. Flujo de ejecución

1. `main.cxx` crea `Application`, ejecuta `go()` y presenta errores fatales en
   el log de Ogre y en la salida estándar.
2. `Application` registra las ubicaciones de recursos, inicializa plugins y
   el `SceneManager`, y carga `fps.scenepp`.
3. La escena proporciona la cámara, la luz asociada al jugador, el suelo, el
   techo y los nodos placeholder del enemigo y el proyectil.
4. `Maze` lee `maze.txt`, convierte sus celdas transitables y paredes entre
   coordenadas de grilla y coordenadas del mundo, y genera bloques de pared.
5. `Model` construye el mundo de Bullet y recorre los nodos que tienen datos
   físicos de escena para crear los cuerpos correspondientes.
6. En cada frame, `Application` avanza la física, actualiza al jugador, decide
   el siguiente movimiento del enemigo, actualiza sus presentaciones y procesa
   lanzamiento, colisiones, derrota y HUD.
7. Si el jugador toca al enemigo mientras no está aturdido, `Game` cambia a
   `LOST`; `LoseScreen` muestra las acciones de repetir o salir. El reinicio
   coordina los métodos `reset` de los componentes.

## 4. Responsabilidad por archivo

### Raíz y construcción

| Archivo | Responsabilidad |
|---|---|
| `.gitignore` | Excluye compilados, directorios de build, logs, configuración local y archivos `*.obj`. Esta última regla también excluye el modelo `bunny.obj` descrito abajo. |
| `CMakeLists.txt` | Define el proyecto C++23, busca OGRE, Bullet y módulos VTK, y añade `apps`, `lib/pujOgre` y `plugins` al build. |
| `apps/CMakeLists.txt` | Incorpora la aplicación `Laberinto`. |
| `README.md` | README actual mínimo; solo contiene el título del proyecto. |
| `CAMBIOS_THROWABLE.md` | Resume decisiones, controles y comportamiento del proyectil, aturdimiento, HUD y derrota. |
| `GUIA_ESTUDIO_CAMBIOS.md` | Guía de estudio de cambios recientes, con orden de lectura y responsabilidades de varios módulos. |

### Aplicación `apps/Laberinto`

| Archivo | Responsabilidad |
|---|---|
| `apps/Laberinto/CMakeLists.txt` | Define el ejecutable `pujOgre_Laberinto`, sus fuentes, bibliotecas y la copia de recursos a `Laberinto_resources` en el directorio de build. |
| `Application.h` | Declara la clase que integra `OgreBites::ApplicationContext` e `InputListener`; mantiene las instancias de los sistemas y define los puntos de configuración y coordinación. |
| `Application.cxx` | Implementa carga de plugins/recursos/escena, configuración del `SceneManager` y la cámara, eventos de teclado/ratón, ciclo por frame, colisiones, derrota y reinicio. Es el coordinador, no la implementación detallada de cada sistema. |
| `main.cxx` | Punto de entrada. Arranca la aplicación y captura excepciones estándar para reportarlas. |
| `Maze.h` | Declara la cuadrícula, `Cell`, conversiones grilla/mundo y operaciones de navegación, visibilidad y colisión. |
| `Maze.cxx` | Lee el mapa, localiza `S`/`E`, calcula vecinos y rutas, comprueba línea de visión/impactos y construye paredes como cubos de Ogre. |
| `Model.h` | Declara la capa física basada en `Ogre::Bullet::DynamicsWorld` y su ciclo de actualización. |
| `Model.cxx` | Lee propiedades físicas de `userData`, crea el mundo Bullet, recorre la escena para registrar cuerpos y avanza la simulación. También conserva una operación auxiliar `shoot`. |
| `Player.h` | Declara el jugador como listener de input, junto con acceso a posición/dirección, dimensiones, reinicio y solicitud de lanzamiento. |
| `Player.cxx` | Implementa la cápsula de Bullet, sincronización entre cuerpo y nodo/cámara, movimiento WASD/flechas, giro con ratón y entrada para lanzar. |
| `Game.h` | Declara el estado de alto nivel `PLAYING`/`LOST` y la comprobación de contacto jugador-enemigo. |
| `Game.cxx` | Implementa la intersección entre el volumen del jugador y los bounds del enemigo; el contacto no pierde si el enemigo está aturdido. |
| `Enemy.h` | Declara estados, temporizadores, objetivos, rutas de patrulla y parámetros de detección/aturdimiento. |
| `Enemy.cxx` | Implementa la máquina de estados `PATROL`, `CHASE`, `SEARCH` y `STUNNED`, además de selección de waypoints y memoria de la última celda del jugador. |
| `EnemyPresentation.h` | Declara el enlace entre el estado lógico del enemigo y su nodo visual de Ogre. |
| `EnemyPresentation.cxx` | Aplica textura y orientación al modelo, interpola movimiento entre celdas y expone posición y bounds para colisiones. |
| `Throwable.h` | Declara los estados `IDLE_HELD`, `IN_FLIGHT` e `INACTIVE`, su física, temporizadores y consultas de intersección. |
| `Throwable.cxx` | Implementa el cuerpo esférico Bullet, lanzamiento, gravedad, velocidad, vida útil, cooldown y desactivación. |
| `ThrowablePresentation.h` | Declara la actualización visual del nodo del proyectil. |
| `ThrowablePresentation.cxx` | Sitúa el proyectil frente al jugador mientras está disponible y actualiza/oculta su nodo según el estado. |
| `CooldownHud.h` | Declara el overlay del cooldown, sus dígitos segmentados y barra de progreso. |
| `CooldownHud.cxx` | Construye y actualiza el HUD; muestra el tiempo restante y cambia el color según disponibilidad. |
| `LoseScreen.h` | Declara el overlay de derrota y las acciones `REPLAY`/`EXIT`. |
| `LoseScreen.cxx` | Dibuja el fondo, título, botones y fuente de píxeles; convierte clics en acciones del juego. |
| `OverlayMaterial.h` | Helper inline que genera materiales OGRE con una textura RGBA 1×1 para que los colores de overlays sean explícitos y no dependan de iluminación 3D. |

### Recursos de `apps/Laberinto/resources`

| Archivo | Responsabilidad |
|---|---|
| `fps.scenepp` | Escena declarativa: cámara, jugador y sus propiedades, ambiente/fog, luz, planos de suelo/techo, placeholders y parámetros del enemigo/proyectil. |
| `materials.material` | Materiales Ogre para suelo, techo, paredes, placeholder del enemigo y proyectil. El material `Wall` actual es rojo oscuro, sin textura, y recibe iluminación de escena. |
| `maze.txt` | Mapa ASCII: `#` representa pared, `.` espacio transitable y `S`/`E` marcan inicio/salida. `Maze` analiza la grilla. |
| `floor.png` | Textura del plano de suelo. |
| `ceiling.png` | Textura del plano de techo. |
| `wall.png` | Textura antigua de pared; ya no está referenciada por el material `Wall` actual. |
| `oxido.png` | Textura aplicada al modelo placeholder del enemigo. |
| `bunny.obj` | Malla Wavefront usada por `MonsterPlaceholder`. **Está presente localmente, pero `.gitignore` la excluye con `*.obj`; no aparece en los archivos versionados.** Una copia limpia del repositorio podría no tener este recurso. |

### Biblioteca `lib/pujOgre`

| Archivo | Responsabilidad |
|---|---|
| `lib/pujOgre/CMakeLists.txt` | Construye la biblioteca compartida `pujOgre` y enlaza Ogre, OgreBites y las bibliotecas VTK encontradas. |
| `FPSCamera.h` | API de una cámara FPS reutilizable: configuración de movimiento, gravedad, salto, límites y entrada. |
| `FPSCamera.cxx` | Implementa el control de cámara, orientación, movimiento y ajuste a los bounds del suelo. |
| `vtkParametricExpression.h` | Declara una superficie paramétrica VTK definida mediante expresiones de `u` y `v`, constantes y normales opcionales. |
| `vtkParametricExpression.cxx` | Compila y evalúa las expresiones de punto/normal usando `vtkFunctionParser`. |
| `vtkPolyData2OgreMesh.h` | Declara conversores entre `vtkPolyData` y mallas Ogre. |
| `vtkPolyData2OgreMesh.cxx` | Triangula datos VTK, transfiere posiciones/normales/UV a buffers Ogre, calcula bounds y crea mallas manuales. |

### Plugins

| Archivo | Responsabilidad |
|---|---|
| `plugins/CMakeLists.txt` | Añade los plugins `DotPPScene` y `VTKCodec`. |
| `plugins/DotPPScene/CMakeLists.txt` | Construye el plugin de carga de escenas `.scenepp`. |
| `plugins/DotPPScene/DotPPSceneLoader.h` | Declara el loader de escena, procesamiento de nodos/luces/cámaras/entidades/planos y registro del codec como plugin Ogre. |
| `plugins/DotPPScene/DotPPSceneLoader.cxx` | Parsea el XML `.scenepp` con pugixml y crea objetos, propiedades de usuario, ambiente, luces, cámaras y mallas para Ogre. |
| `plugins/DotPPScene/ParametricSurfaceBuilder.h` | Declara la conversión de elementos `<parametricSurface>` a geometría VTK y la combinación de definiciones/sobrescrituras XML. |
| `plugins/DotPPScene/ParametricSurfaceBuilder.cxx` | Construye toros, elipsoides, superelipsoides o superficies definidas por expresiones; configura rangos, resolución y normales. |
| `plugins/DotPPScene/dotscene.dtd` | DTD que describe la estructura XML de escenas y sus nodos, propiedades, geometrías, luces y animaciones. |
| `plugins/DotPPScene/pugiconfig.hpp` | Configuración de compilación de la copia integrada de pugixml. |
| `plugins/DotPPScene/pugixml.hpp` | Parser XML pugixml incluido para leer los documentos de escena. |
| `plugins/VTKCodec/CMakeLists.txt` | Construye el plugin que conecta codecs de malla con la biblioteca `pujOgre`. |
| `plugins/VTKCodec/VTKCodec.cxx` | Registra decoders de Ogre para `.obj`, `.vtk` y `.vtp`; usa VTK para leer datos y `pujOgre` para producir la malla Ogre. |

## 5. Configuración y límites que conviene saber

- **Cambiar el trazado del laberinto:** editar `apps/Laberinto/resources/maze.txt`.
- **Cambiar cámara, entidades o valores de juego:** editar las propiedades de
  los nodos en `apps/Laberinto/resources/fps.scenepp`.
- **Cambiar apariencia:** editar `apps/Laberinto/resources/materials.material`
  y los PNG referenciados por esos materiales.
- **Cambiar reglas de navegación/colisión:** revisar `Maze`; cambiar decisiones
  del enemigo en `Enemy`; la coordinación general permanece en `Application`.
- El mapa contiene una marca de salida `E`, y `Maze` la guarda; el estado
  `Game` actual solo declara `PLAYING` y `LOST`, así que no debe presentarse
  como una condición de victoria ya implementada.
- La escena contiene parámetros de objetos (`userData`); `Application` y
  `Model` los consumen para configurar física y jugabilidad.
- `Export.h` y los artefactos bajo `build/` son generados por CMake; no son
  archivos fuente mantenidos a mano.

## 6. Compilación

Con las dependencias de Ogre3D, Bullet y VTK disponibles:

```sh
cmake -S . -B build
cmake --build build --target pujOgre_Laberinto -j2
```

El target copia los recursos de la aplicación al directorio de build y produce
el ejecutable `pujOgre_Laberinto`.
