# Study guide: monster, throwable, and cooldown HUD

This inventory covers files changed on the current branch compared with
`origin/main` (`feature/gameover-screen`). It includes monster navigation/FSM,
throwable physics, cooldown HUD, and game-over/replay behavior.

## Suggested study order

1. `apps/Laberinto/resources/fps.scenepp` — scene nodes and tunable values.
2. `apps/Laberinto/Maze.h` / `Maze.cxx` — grid coordinates, walkability,
   line-of-sight, path selection, and projectile collision.
3. `apps/Laberinto/Enemy.h` / `Enemy.cxx` — enemy FSM and stun timer.
4. `apps/Laberinto/EnemyPresentation.h` / `.cxx` — visual movement and
   orientation of the monster node.
5. `apps/Laberinto/Throwable.h` / `.cxx` — throwable state, Bullet rigid body,
   lifetime, and cooldown.
6. `apps/Laberinto/ThrowablePresentation.h` / `.cxx` — throwable visibility
   and Ogre node updates.
7. `apps/Laberinto/CooldownHud.h` / `.cxx` — bottom-right cooldown display.
8. `apps/Laberinto/Player.h` / `.cxx` and `Application.h` / `.cxx` — input
   requests and per-frame coordination.
9. `apps/Laberinto/Model.cxx` — opt-out for scene nodes that should not get a
   static physics body.
10. `apps/Laberinto/Game.h` / `Game.cxx` — playing/lost state and overlap rules.
11. `apps/Laberinto/LoseScreen.h` / `LoseScreen.cxx` — lose screen and button handling.
12. `apps/Laberinto/OverlayMaterial.h`,
    `apps/Laberinto/resources/materials.material` and
    `apps/Laberinto/CMakeLists.txt` — visual material and build registration.

## Modified existing files

| File | What changed |
|---|---|
| `.gitignore` | Ignores PDF files in the local repository. |
| `CMakeLists.txt` | Enables the VTK modules needed to load Wavefront OBJ meshes and generate texture coordinates. |
| `apps/Laberinto/Application.cxx` | Initializes game systems; chooses a walkable enemy spawn behind the player and coordinates frame updates, collisions, loss, replay, and exit. |
| `apps/Laberinto/Application.h` | Owns the game state, lose screen, enemy and throwable, presentation components, and cooldown HUD. |
| `apps/Laberinto/CMakeLists.txt` | Registers the new C++ source files in the game target. |
| `apps/Laberinto/Maze.cxx` | Implements world/grid conversion, walkability and neighbor queries, patrol points, route steps, line-of-sight, and projectile collision against maze bounds, walls, floor, and ceiling. |
| `apps/Laberinto/Maze.h` | Declares the `Cell` type and grid/navigation/collision queries, including directional walkable-neighbor selection. |
| `apps/Laberinto/Model.cxx` | Skips static physics creation for nodes whose `physics_enabled` scene property is false. |
| `apps/Laberinto/Player.cxx` / `Player.h` | Reports mouse-click/Q throws, exposes position and dimensions for gameplay, supports resetting the Bullet body, and gates input during loss. |
| `apps/Laberinto/resources/fps.scenepp` | Defines the monster and throwable placeholders, with launch, cooldown, stun, and physics settings in scene data. |
| `apps/Laberinto/resources/materials.material` | Defines the placeholder materials, including the monster's oxide texture. |
| `plugins/VTKCodec/VTKCodec.cxx` | Adds OBJ loading through VTK and generates spherical UVs when importing the monster mesh. |
| `apps/Laberinto/Enemy.cxx` / `Enemy.h` | Adds enemy FSM reset support for replay. |
| `apps/Laberinto/EnemyPresentation.cxx` / `EnemyPresentation.h` | Binds the monster mesh to its oxide texture, applies a 180-degree Y rotation, exposes world bounds for contact checks, and restores its spawn transform on replay. |
| `apps/Laberinto/Throwable.cxx` / `Throwable.h` | Adds reset support for replay, removing the body and clearing cooldown. |

## New files

| File | Purpose |
|---|---|
| `CAMBIOS_THROWABLE.md` | Implementation notes and control/configuration summary for the throwable and stun behavior. |
| `apps/Laberinto/Enemy.cxx` | Implements patrol, chase, search, and stunned state behavior. |
| `apps/Laberinto/Enemy.h` | Declares enemy state, targets, timers, and update/stun interface. |
| `apps/Laberinto/EnemyPresentation.cxx` | Moves and orients the monster’s Ogre scene node independently from AI decisions. |
| `apps/Laberinto/EnemyPresentation.h` | Declares the monster presentation interface. |
| `apps/Laberinto/Throwable.cxx` | Implements throwable state, Bullet sphere body, trajectory, lifetime, cooldown, and proximity checks. |
| `apps/Laberinto/Throwable.h` | Declares throwable state and simulation interface. |
| `apps/Laberinto/ThrowablePresentation.cxx` | Updates the throwable scene node’s position, orientation, and visibility. |
| `apps/Laberinto/ThrowablePresentation.h` | Declares the throwable presentation interface. |
| `apps/Laberinto/CooldownHud.cxx` | Builds and updates the numeric, segmented cooldown timer and progress bar. |
| `apps/Laberinto/CooldownHud.h` | Declares the cooldown HUD interface and overlay elements. |
| `apps/Laberinto/Game.cxx` | Implements the PLAYING/LOST state and player/monster overlap rule, bypassing contact while stunned. |
| `apps/Laberinto/Game.h` | Declares the high-level game state and contact update interface. |
| `apps/Laberinto/LoseScreen.cxx` | Builds the lose overlay, pixel-font heading and clickable REPLAY/EXIT buttons with explicit colors, centered button placement, and matching hit regions. |
| `apps/Laberinto/LoseScreen.h` | Declares the lose-screen presentation and click action interface. |
| `apps/Laberinto/OverlayMaterial.h` | Creates explicit RGBA texture-backed materials for UI overlay colors. |
| `apps/Laberinto/resources/oxido.png` | Texture asset used by the monster placeholder material. |

## Key configuration and controls

- **Throw:** left mouse click or `Q`.
- **Throwable lifetime:** 5 seconds maximum.
- **Throwable cooldown:** 60 seconds from launch.
- **Enemy stun duration:** 30 seconds after a valid hit.
- **Lose condition:** the player capsule touches the enemy mesh while it is not stunned.
- **Replay:** resets player, monster and throwable state; **Exit:** closes the game.
- Placeholder mesh, material, and gameplay tuning values are declared in
  `apps/Laberinto/resources/fps.scenepp`.
