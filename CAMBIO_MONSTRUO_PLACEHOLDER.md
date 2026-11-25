# Cambio: placeholder del monstruo

## Archivos modificados

- `apps/Laberinto/resources/fps.scenepp`
- `apps/Laberinto/resources/materials.material`
- `apps/Laberinto/Application.h` y `Application.cxx`
- `apps/Laberinto/Enemy.h` y `Enemy.cxx`
- `apps/Laberinto/EnemyPresentation.h` y `EnemyPresentation.cxx`
- `apps/Laberinto/Maze.h` y `Maze.cxx`
- `apps/Laberinto/Model.cxx`
- `apps/Laberinto/CMakeLists.txt`

## Ubicacion

- Se agrego el nodo `MonsterPlaceholder` al archivo de escena.
- Posicion inicial: `(0, 0.8, 2)` en coordenadas del mundo.
- La posicion corresponde a una celda abierta del laberinto, calculada con el
  sistema de coordenadas usado por `Maze::cellToWorld`.

## Representacion

- El placeholder es un super-elipsoide con parametros cercanos a un cubo:
  radio de 0.8 unidades en cada eje y valores `n1/n2` de `0.1`.
- Se agrego el material `MonsterPlaceholder` con color rojo para distinguirlo.
- El nodo tiene `physics_enabled=false`: la geometria se mueve visualmente sin
  dejar atras un collider estatico desincronizado.

## Enemigo y responsabilidades

- `Enemy` contiene los estados `PATROL`, `CHASE` y `SEARCH`, la seleccion de
  objetivos y el temporizador de investigacion. Persigue la ultima celda conocida
  cuando pierde al jugador y vuelve a patrullar al terminar la busqueda.
- `Maze` contiene la conversion mundo/grilla, las celdas transitables, vecinos,
  puntos de patrulla, busqueda de ruta, seleccion del siguiente paso sin invertir
  la marcha cuando hay alternativas, y el raycast de visibilidad en la grilla.
- `EnemyPresentation` aisla el movimiento y la orientacion del nodo Ogre del
  calculo de estados y rutas.
- `Application` solo inicializa el enemigo y despacha el tiempo/posicion del
  jugador cada frame; no contiene logica de IA ni de navegacion.

## Alcance y validacion

- No se agregaron FSM, vision, ataques, items ni reglas de victoria/derrota del
  diseno final del PDF. El FSM implementado aqui es el solicitado para esta
  iteracion: patrulla, persecucion e investigacion.
- Se conserva la escena y el ciclo de fisica existentes; el placeholder visual
  no se convierte en un cuerpo fisico dinamico.
- `cmake --build build -j2` compila el proyecto.

## Ambientacion de horror

- Se redujo la luz ambiental y se cambio la niebla a tonos azul-negro para
  conservar sombras y visibilidad parcial.
- Se corrigio la luz cercana al jugador: ahora es un foco dirigido con alcance
  y atenuacion moderados, en vez de iluminar en todas direcciones como una luz
  puntual.
- Se redujo el brillo especular de suelo, paredes y techo, se les dio un matiz
  frio y se mantuvo el material rojo del enemigo visible bajo el foco.
- Se sustituyo el fondo gris claro del viewport por un tono casi negro, coherente
  con el color de la niebla.
- Sin cambios de arquitectura, clases, fisica o logica de juego.
