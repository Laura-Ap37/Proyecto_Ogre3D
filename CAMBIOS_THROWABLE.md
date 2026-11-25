# Throwable y aturdimiento del enemigo

## Cambios realizados

- `apps/Laberinto/resources/fps.scenepp`: define `ThrowablePlaceholder` con
  geometria parametrica, material, escala, posicion inicial y parametros
  editables de lanzamiento. Tambien define radio y duracion del aturdimiento
  en los datos de escena de `MonsterPlaceholder`.
- `apps/Laberinto/resources/materials.material`: agrega el material del throwable.
- `apps/Laberinto/Throwable.h` y `Throwable.cxx`: separa estado held/in-flight/
  inactive, trayectoria y colisiones con un cuerpo rigido de Bullet, gravedad,
  velocidad, vida, cooldown y deteccion de proximidad sobre el segmento recorrido.
- `apps/Laberinto/ThrowablePresentation.h` y `.cxx`: enlaza el nodo de escena,
  muestra el throwable en la mano, y actualiza su posicion/orientacion o lo oculta.
- `apps/Laberinto/Player.h` y `.cxx`: la tecla `Q` genera una solicitud de
  lanzamiento y expone posicion de ojos/direccion de mira, sin contener la
  simulacion del throwable.
- `apps/Laberinto/Maze.h` y `.cxx`: agrega colision de segmento con paredes,
  limites, piso y techo usando la grilla existente.
- `apps/Laberinto/Enemy.h` y `.cxx`: agrega `STUNNED` y temporizador de duracion
  configurable. Al terminar, vuelve a PATROL o SEARCH; si fue interrumpido en
  CHASE, retoma en SEARCH usando la ultima celda conocida.
- `apps/Laberinto/EnemyPresentation.h` y `.cxx`: pausa el movimiento visual al
  estar aturdido y expone la posicion visual para la coordinacion de impactos.
- `apps/Laberinto/Application.h` y `.cxx`: despacha el lanzamiento y coordina
  colision con Maze y el radio de stun del enemigo.
- `apps/Laberinto/CMakeLists.txt`: registra las dos nuevas unidades `.cxx`.

## Controles y alcance

- Haz clic izquierdo o pulsa `Q` para lanzar el throwable en la direccion de la
  camara.
- La esfera usa fisica de Bullet y la consulta de grilla termina el lanzamiento
  al impactar paredes o piso.
- Al impactar al monstruo, lo deja aturdido durante 30 segundos.
- Desaparece tras 5 segundos como maximo y vuelve a estar disponible al cumplirse
  el cooldown de 60 segundos contado desde el lanzamiento. Ambos valores se
  configuran desde el nodo de escena.
- Se mantuvieron separadas la simulacion, la presentacion y la orquestacion; no
  se cambio el flujo base de movimiento/colisiones del jugador ni la fisica del
  maze.

## Derrota y replay

- El nodo `MonsterPlaceholder` usa la malla Wavefront OBJ `bunny.obj` y el
  material `MonsterPlaceholder` aplica `oxido.png`. Como la malla no trae UVs,
  el codec VTK genera coordenadas esfericas de textura al cargar el OBJ.
- `EnemyPresentation::build` carga y asigna explicitamente `oxido.png` al
  material de las entidades del monstruo, con la etapa de textura configurada
  para reemplazar el color de reserva BaseWhite. La orientacion local del mesh
  se corrige 180 grados sobre el eje Y al inicio y al moverlo.
- El enemigo aparece en la celda transitable adyacente que queda detras de la
  orientacion inicial del jugador; `Maze` elige la celda valida y `Application`
  posiciona alli el nodo. El replay reutiliza esa misma celda de spawn.
- `Game.h` y `Game.cxx` controlan el estado PLAYING/LOST y detectan el contacto
  entre la capsula del jugador y los limites de mundo del mesh del monstruo.
- El contacto durante STUNNED no causa derrota, de modo que el jugador puede
  atravesar al monstruo aturdido.
- `LoseScreen.h` y `LoseScreen.cxx` implementan una pantalla superpuesta con los
  botones REPLAY y EXIT, incluyendo la interaccion por clic.
- `OverlayMaterial.h` crea materiales OGRE respaldados por texturas RGBA de 1x1
  para la pantalla de derrota y el HUD, de modo que el color no dependa de la
  interpretacion del color de vertices del overlay.
- Al perder, se pausan la simulacion y los controles, y se libera el mouse para
  usar los botones. REPLAY restaura la posicion/velocidad del jugador, el estado
  y posicion inicial del enemigo, y el throwable con cooldown reiniciado.
- `Player`, `Enemy`, `EnemyPresentation` y `Throwable` agregan operaciones
  pequeñas de reset para que el flujo de replay no replique su logica dentro de
  `Application`.
- Se registraron `Game.cxx` y `LoseScreen.cxx` en el target de CMake.
- Los botones se centraron en la pantalla y sus regiones de clic se actualizaron
  para coincidir con la nueva posicion.

## HUD del cooldown

- Se agrego `CooldownHud` como componente de presentacion OGRE independiente.
- Muestra el tiempo restante mediante digitos segmentados y una barra en la
  esquina inferior derecha; verde indica disponible y ambar indica enfriamiento.
- El HUD lee la duracion y el tiempo restante directamente de `Throwable`.
- Se habilito el listener de overlays en el SceneManager y se registro
  `CooldownHud.cxx` en CMake.
