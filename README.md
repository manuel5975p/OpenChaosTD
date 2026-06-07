# OpenChaosTD
An open-map 2D Tower Defense game written in C++ with raylib. Towers can be placed anywhere on a grid and enemies dynamically pathfind around them. Still a work in progress.

## Building

### Prerequisites
* **CMake** 3.22 or newer
* **C++20 compiler** (GCC, Clang, MSVC)
* **Git** (for fetching dependencies via FetchContent)
* **Emscripten SDK** (web builds only)
* **Python 3.8+** (web builds only)

### Desktop (Windows, macOS, Linux)
```bash
mkdir build
cd build
cmake ..
cmake --build .
```
The binary is output to `build/bin/`.

### Web (WebAssembly)
```bash
cd tools
./build_web.sh
```
A local HTTP server starts on port 8000. The URL will be shown in the terminal.

## Dependencies
All fetched automatically via CMake FetchContent — no manual installation needed.

| Library | Version | Purpose |
|---|---|---|
| [raylib](https://github.com/raysan5/raylib) | 6.0 | Window, rendering, input, audio |
| [nlohmann/json](https://github.com/nlohmann/json) | 3.11.3 | JSON parsing for data files |

## Modding
Towers, enemies, and waves are fully data-driven — no recompile needed to tweak balance or add content.
Each JSON file under `data/` has a companion `.md` schema doc:

| File | Docs | Configures |
|---|---|---|
| `data/towers.json`  | [towers.md](data/towers.md)   | Tower stats and attack/effect modules (incl. the armor chip-damage floor) |
| `data/enemies.json` | [enemies.md](data/enemies.md) | Enemy stats and modules (armor, shield, regen, split spacing, immunities, upgrades) |
| `data/waves.json`   | [waves.md](data/waves.md)     | Procedural wave generator: budget scaling models, boss/upgrade cadence, enemy pool |

## Media
*(Add a screenshot or GIF here)*

## Project Structure
```
OpenChaosTD/
├── resources/
│   ├── textures/               - Sprites for towers, enemies and tiles
│   ├── music/                  - Streaming background music (OGG recommended)
│   └── sounds/                 - One-shot sound effects
│
├── config/
│   ├── settings.json           - Window resolution, FPS, HUD scale, title, audio volumes
│   └── keybindings.json        - Input action bindings (rebindable)
│
├── data/
│   ├── gameplay.json           - Starting lives, gold, sell rate, auto-spawn delay
│   ├── towers.json             - Tower type definitions (stats, modules, description)
│   ├── enemies.json            - Enemy type definitions (stats, modules, description)
│   ├── waves.json              - Procedural wave generator: budget scaling, boss/upgrade cadence, enemy pool
│   ├── particle_effects.json   - Named particle emitter presets
│   └── *.md                    - Modder schema docs for towers/enemies/waves JSON
│
└── src/
    ├── main                    - Entry point
    ├── game                    - Game loop, state machine, manager accessors
    ├── game_config             - Window/display settings loaded from JSON
    │
    ├── engine/                 Reusable engine infrastructure — see engine/engine.md
    │
    ├── factory/                Data-driven entity construction from JSON
    │   ├── tower_factory       - Builds Tower instances from towers.json
    │   ├── enemy_factory       - Builds Enemy instances from enemies.json
    │   └── emitter_presets     - Loads named EmitterDesc presets from particle_effects.json
    │
    ├── hud/                    In-game HUD elements
    │   ├── hud                 - HUD base class: lifecycle, visibility, scaling, panel helpers
    │   ├── button              - Shared UI primitive (hit-test + draw)
    │   ├── score_hud           - Top bar: lives, gold, wave info, start/auto buttons
    │   ├── tower_hud           - Bottom panel: tower selection with cost and icon
    │   ├── tower_info_hud      - Floating panel: stats and sell button for selected/hovered tower
    │   └── event_log           - Top-left message log with timed fade-out
    │
    ├── states/                 Game screen state machine
    │   ├── game_state          - Abstract base (ProcessInput, Update, Draw)
    │   ├── menu_state          - Main menu
    │   ├── play_state          - Active gameplay
    │   └── end_state           - Victory / game over screen
    │
    ├── world/                  Entity definitions and data
    │   ├── game_data           - Runtime world state + starting values (JSON)
    │   ├── tower               - Tower entity (position, stats, description, modules)
    │   ├── tower_modules       - Attack/Passive + ArmorPierce, Slow, Burn, ArmorShred, Weakness, Stun, Crit, RampUp
    │   ├── tower_vfx           - Visual effect descriptors attached to towers
    │   ├── enemy               - Enemy entity (position, health, description, effects, modules)
    │   ├── enemy_modules       - BaseStats + Regeneration, Armor, Immune, Shield, Split
    │   ├── attack              - Attack object (origin, targets, payload, visual)
    │   ├── effect              - Status effect (Burn, Slow) with duration
    │   ├── vfx_effect          - Active in-flight visual effect (trail, impact)
    │   ├── map                 - Grid, nest/core placement, path construction
    │   └── tile                - Tile type, walkable/buildable flags
    │
    └── systems/                Per-frame game logic
        ├── pathfinder          - BFS Node type (distance, predecessor)
        ├── wave_manager        - Procedural budget-based wave generation, auto-spawn, victory detection
        ├── world_system        - Placement, spawning, game-over checks
        ├── tower_system        - Cooldowns, targeting, attack creation and resolution
        ├── enemy_system        - Movement, status effects, module ticking
        └── render_system       - Drawing map, entities, attacks, UI
```

## License
MIT — see [LICENSE](LICENSE) for details.
