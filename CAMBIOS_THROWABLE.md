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
- No se modifico la arquitectura de clases existente ni el flujo del jugador,
  fisica base o maze.

## HUD del cooldown

- Se agrego `CooldownHud` como componente de presentacion OGRE independiente.
- Muestra el tiempo restante mediante digitos segmentados y una barra en la
  esquina inferior derecha; verde indica disponible y ambar indica enfriamiento.
- El HUD lee la duracion y el tiempo restante directamente de `Throwable`.
- Se habilito el listener de overlays en el SceneManager y se registro
  `CooldownHud.cxx` en CMake.
