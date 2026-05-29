# Engine

Reusable infrastructure shared across all game states and systems. Located under `src/engine/` and split into four subfolders by role.

```
engine/
├── core/       Windowing, rendering, input, resource loading
├── features/   Optional gameplay-adjacent systems (particles, sound)
├── util/       File I/O and profiling helpers
└── lib/        Generic data structures with no engine dependencies
```

---

## core/

### Resources

Loads and caches raylib assets by string key. Supports textures, sounds, fonts, and music. Assets are freed in `Shutdown()`, called automatically by the destructor.

```cpp
m_resources.SetAssetPath(path);

// Single file
m_resources.LoadTexture("tower_zapper", "textures/tower_zapper.png");
Texture2D& tex = m_resources.GetTexture("tower_zapper");

// Bulk load — key = filename stem for each file in the directory
m_resources.LoadTexturesFromDir("textures");
m_resources.LoadMusicFromDir("music");
```

Retrieval throws if the key is not found. Use `HasTexture` / `HasSound` / etc. to guard optional lookups.

OGG Vorbis is the recommended format for music streaming; WAV is best used with `LoadSound` for short one-shot effects.

---

### Screen

Renders the game into a fixed virtual resolution (`RenderTexture2D`) and scales it to fill the window with letterboxing. The virtual size is set once at `Init()` and never changes; only the destination rectangle recalculates on resize.

```cpp
// game loop
if (IsWindowResized()) m_screen.OnResize();
m_screen.BeginFrame();
    // all game drawing here
m_screen.EndFrame();
```

`GetVirtualMouse()` converts real screen mouse coordinates into virtual game coordinates, accounting for letterbox offset and scale. All input should use this rather than raylib's `GetMousePosition()` directly.

---

### Input

Action-based input that maps named actions to keyboard keys via `keybindings.json`. Unknown action names return `false` silently.

Mouse buttons are queried directly via raw accessors — they are not part of the binding system and are not user-remappable.

```cpp
// keyboard action querying
if (input.IsPressed("Cancel")) { ... }

// raw mouse button querying
if (input.IsMousePressed(MOUSE_LEFT_BUTTON))  { ... }
if (input.IsMouseDown(MOUSE_RIGHT_BUTTON))    { ... }
if (input.IsMouseReleased(MOUSE_LEFT_BUTTON)) { ... }
float wheel = input.GetMouseWheelDelta();
```

Mouse input consumption prevents clicks from passing through HUD elements to the world:

```cpp
input.ConsumeMouseInput();        // called by HUD
input.IsMouseInputConsumed();     // checked by world interactions
```

Consumed state resets at the start of each frame in `Update()`.

---

## features/

### ParticleSystem

CPU particle system backed by a fixed-capacity `ObjectPool<Particle>` (2048 slots). Owned by `Game` alongside the other core managers — not part of game state.

`EmitterDesc` is the public API for configuring a burst. `Particle` is an internal runtime type.

```cpp
// Spawn a burst at a world position
game.GetParticles().Emit(position, desc);

// With base velocity so particles follow a moving emitter (e.g. status effects on enemies)
game.GetParticles().Emit(position, desc, enemyVelocity);

// Drive from the game loop
game.GetParticles().Tick(dt);
game.GetParticles().Draw();

// Clear all live particles (call alongside GameData::Reset)
game.GetParticles().Clear();
```

`EmitterDesc` fields — all angles in degrees:

| Field | Default | Meaning |
|---|---|---|
| `color` / `endColor` | WHITE / transparent | Start and end tint, lerped over lifetime |
| `count` | 0 | Particles per burst; 0 = disabled |
| `speed` / `speedVariance` | 50 / 20 | Ejection speed ± random |
| `angle` | 0 | Centre direction (0=right, 90=down, 180=left, 270=up) |
| `spread` | 360 | Total arc of the spawn cone in degrees |
| `lifetime` | 0.2 | Seconds each particle lives |
| `size` / `endSize` | 3 / 0 | Radius at birth and death, lerped over lifetime |

---

### SoundSystem

One-track music streamer plus fire-and-forget sound effects. Owned by `Game`, initialised after resources are loaded. `Tick` must be called every frame to feed the active music stream.

```cpp
// Startup (called once in Game::LoadResources after LoadMusicFromDir)
game.GetSoundSystem().Init(m_resources);

// Music — one track at a time; starting a new track stops the previous one
game.GetSoundSystem().PlayMusic("openchaostd_main");
game.GetSoundSystem().PauseMusic();
game.GetSoundSystem().ResumeMusic();
game.GetSoundSystem().StopMusic();

// Sound effects — fire-and-forget, played at the current sfx volume
game.GetSoundSystem().PlaySfx("explosion");

// Volume — [0.0, 1.0]; music volume applies immediately to the active track
game.GetSoundSystem().SetMusicVolume(0.8f);
game.GetSoundSystem().SetSfxVolume(0.5f);

// Game loop — must run every frame
game.GetSoundSystem().Tick(dt);
```

---

## util/

### JsonStore

Cross-platform JSON read/write backed by nlohmann/json. The root path is set once and all subsequent paths are resolved relative to it.

```cpp
m_jsonStore.SetRootPath(projectRoot);
auto j = m_jsonStore.Load("data/towers.json");
m_jsonStore.Save("data/save.json", j);
bool exists = m_jsonStore.Exists("data/config.json");
```

Used by all factories and config types. On web builds, path resolution accounts for the Emscripten virtual filesystem.

---

### Profiler

Named scope profiler using a rolling window of samples. Tracks average, last, and peak frame times per scope. Results are printed to stdout on shutdown.

```cpp
m_profiler.Begin("Update");
    // ...
m_profiler.End("Update");

double avg = m_profiler.GetAvgMs("Update");
```

The window size defaults to 120 frames (2 s at 60 fps).

---

## lib/

Header-only data structures with no engine or game dependencies.

### DenseSlotMap

Stable-ID container with dense storage optimised for fast iteration. Handles are stable across insertions and removals. Used for towers and enemies.

```cpp
DenseSlotMap<Tower> towers;
auto key = towers.Insert(tower);
Tower& t = towers.Get(key);
towers.Remove(key);
for (auto& t : towers) { ... }
```

### ObjectPool

Fixed-capacity pool with O(1) allocate and release. Release uses swap-and-pop, so iteration order is not stable. Used by `ParticleSystem`.

### Grid2D

Resizable 2D array backed by a flat `std::vector`. Used for the tile map and the BFS path mesh.

```cpp
Grid2D<Tile> grid(width, height);
grid.At(x, y) = Tile{...};
```
