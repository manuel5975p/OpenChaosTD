# Core Modules

Engine infrastructure shared across all game states and systems.

---

## AssetManager

Loads and caches raylib assets by string key. Supports textures, sounds, fonts, and music. Assets are freed in `Shutdown()`, called automatically by the destructor.

```cpp
m_assets.SetAssetPath(path);
m_assets.LoadTexture("tower_zapper", "textures/tower_zapper.png");
Texture2D& tex = m_assets.GetTexture("tower_zapper");
```

Retrieval throws if the key is not found. Use `HasTexture` / `HasSound` / etc. to guard optional lookups.

---

## Renderer

Renders the game into a fixed virtual resolution (`RenderTexture2D`) and scales it to fill the window with letterboxing. The virtual size is set once at `Init()` and never changes; only the destination rectangle recalculates on resize.

```cpp
// game loop
if (IsWindowResized()) m_renderer.OnResize();
m_renderer.BeginFrame();
    // all game drawing here
m_renderer.EndFrame();
```

`GetVirtualMouse()` converts real screen mouse coordinates into virtual game coordinates, accounting for letterbox offset and scale. All input should use this rather than raylib's `GetMousePosition()` directly.

---

## InputManager

Action-based input that maps named actions to keyboard keys or mouse buttons via `std::variant<KeyboardKey, MouseButton>`. Unknown action names return `false` silently.

Bindings are set from code defaults first, then overridden by `config/keybindings.json` if present — allowing user rebinding without recompiling.

```cpp
// registering (code defaults)
m_input.AddAction("PlaceTower", MOUSE_LEFT_BUTTON);
m_input.AddAction("Cancel", KEY_ESCAPE);

// querying
if (input.IsPressed("PlaceTower")) { ... }
if (input.IsDown("DragCamera"))    { ... }
float wheel = input.GetMouseWheelDelta();
```

Mouse input consumption prevents clicks from passing through HUD elements to the world:

```cpp
input.ConsumeMouseInput();           // called by HUD
input.IsMouseInputConsumed();        // checked by world interactions
```

Consumed state resets at the start of each frame in `Update()`.

---

## JsonIO

Cross-platform JSON read/write backed by nlohmann/json. The root path is set once and all subsequent paths are resolved relative to it.

```cpp
m_jsonio.SetRootPath(projectRoot);
auto j = m_jsonio.Load("config/towers.json");
m_jsonio.Save("config/save.json", j);
bool exists = m_jsonio.Exists("config/config.json");
```

Used by all factories and config types. On web builds the path resolution accounts for the Emscripten virtual filesystem.

---

## GameConfig

Window and display settings loaded from `config/config.json` at startup, before `InitWindow` is called. Falls back to compiled-in defaults if the file is absent.

```json
{ "gameWidth": 1200, "gameHeight": 1200, "fps": 120, "hudScale": 1.0, "title": "OpenChaos TD" }
```

`ApplyIcon()` generates a procedural 64×64 window icon and must be called after `InitWindow`.

---

## PerformanceMonitor

Named scope profiler using a rolling window of samples. Tracks average, last, and peak frame times per scope. Results are printed to stdout on shutdown.

```cpp
m_monitor.Begin("Update");
    // ...
m_monitor.End("Update");

double avg = m_monitor.GetAvgMs("Update");
```

The window size defaults to 120 frames (2 s at 60 fps).

---

## ParticleSystem

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
