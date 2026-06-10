# OpenChaosTD
An open-map 2D Tower Defense game written in C++ with raylib. Towers can be placed anywhere on a grid and enemies dynamically pathfind around them. Still a work in progress.

## Building

### Prerequisites
* **CMake** 3.22 or newer
* **C++23 compiler** (GCC, Clang, MSVC)
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
| [toml++](https://github.com/marzer/tomlplusplus) | 3.4.0 | TOML parsing for game data files |
| [nlohmann/json](https://github.com/nlohmann/json) | 3.11.3 | JSON parsing for config files (settings, keybindings) |

## Modding
Towers, enemies, and waves are fully data-driven — no recompile needed to tweak balance or add content.
Each TOML file under `data/` has a companion `.md` schema doc under `docs/`:

| File | Docs | Configures |
|---|---|---|
| `data/towers.toml`  | [towers.md](docs/towers.md)   | Tower stats and attack/effect modules (incl. the armor chip-damage floor) |
| `data/enemies.toml` | [enemies.md](docs/enemies.md) | Enemy stats and modules (armor, shield, regen, split spacing, immunities, upgrades) |
| `data/waves.toml`   | [waves.md](docs/waves.md)     | Procedural wave generator: budget scaling models, boss/upgrade cadence, enemy pool |

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
├── datapacks/
│   └── default/
│       ├── pack.toml           - Datapack metadata (name, author, version, description, icon)
│       └── data/
│           ├── gameplay.toml           - Starting lives, gold, sell rate, auto-spawn delay
│           ├── towers.toml             - Tower type definitions (stats, modules, description)
│           ├── enemies.toml            - Enemy type definitions (stats, modules, description)
│           ├── waves.toml              - Procedural wave generator: budget scaling, boss/upgrade cadence, enemy pool
│           └── particle_effects.toml   - Named particle emitter presets
│
├── docs/
│   └── *.md                    - Modder schema docs for towers/enemies/waves TOML
│
└── src/
    ├── main                    - Entry point
    ├── game                    - Game loop, state machine, manager accessors
    ├── game_config             - Window/display settings loaded from JSON
    │
    ├── engine/                 Reusable engine infrastructure — see engine/engine.md
    │
    ├── factory/                Data-driven entity construction from TOML
    │   ├── tower_factory       - Builds Tower instances from towers.toml
    │   ├── enemy_factory       - Builds Enemy instances from enemies.toml
    │   └── emitter_presets     - Loads named EmitterDesc presets from particle_effects.toml
    │
    ├── hud/                    In-game HUD elements
    │   ├── hud                 - HUD base class: visibility, scaling, panel helpers, HudSignal
    │   ├── hud_views           - Read-only view structs PlayingState feeds to the HUDs
    │   ├── status_hud          - Top bar: lives, gold, wave readout, start/auto/speed/waves buttons
    │   ├── tower_build_hud     - Bottom bar: tower selection with icon and cost
    │   ├── tower_info_hud      - Floating panel: stats, upgrade, sell, targeting for selected/hovered tower
    │   ├── wave_hud            - Side panel: next-wave preview (enemy cards + budget)
    │   ├── wave_enemy_card     - One enemy entry in the wave preview panel
    │   ├── event_hud           - Message log with timed fade-out
    │   └── pause_hud           - Pause overlay: resume / restart / main menu
    │
    ├── datapack/               Datapack registry and metadata
    │   ├── datapack            - Datapack metadata struct
    │   └── datapack_registry   - Discovers and lists available datapacks
    │
    ├── states/                 Game screen state machine
    │   ├── game_state          - Abstract base (ProcessInput, Update, Draw)
    │   ├── menu_state          - Main menu
    │   ├── settings_state      - Settings / options menu
    │   ├── play_state          - Active gameplay
    │   ├── end_state           - Victory / game over screen
    │   ├── datapack_select_state - Datapack selection screen
    │   └── particle_editor_state - Live particle emitter editor
    │
    ├── world/                  Entity definitions and data
    │   ├── game_data           - Runtime world state + starting values
    │   ├── tower               - Tower entity: simulation state (position, cost, cooldown, modules)
    │   ├── tower_presentation  - Tower visuals/animation (texture, color, attack style, emitters)
    │   ├── tower_modules       - Attack/Passive + ArmorPierce, Slow, Burn, ArmorShred, Weakness, Stun, Crit, RampUp
    │   ├── tower_upgrade       - Tower upgrade tiers (stat deltas + added modules)
    │   ├── enemy               - Enemy entity: simulation state (position, health, effects, modules)
    │   ├── enemy_presentation  - Enemy visuals (texture, death sound/burst)
    │   ├── enemy_modules       - BaseStats + Regeneration, Armor, Immune, Shield, Split
    │   ├── enemy_upgrade       - Enemy upgrade definition (applied per wave tier)
    │   ├── attack              - Attack object (combat payload + visual, shared lifetime)
    │   ├── effect              - Status effect (Burn, Slow, ArmorShred, Stun, Weakness) with duration
    │   ├── map                 - Grid, nest/core placement, path construction
    │   ├── map_generator       - Procedural map generation
    │   └── tile                - Tile type, walkable/buildable flags, terrain buff modifier
    │
    └── systems/                Per-frame game logic
        ├── pathfinder          - BFS solver over an abstract walkable grid (Pathfinder::Solve -> distance/predecessor mesh)
        ├── wave_manager        - Procedural budget-based wave generation, auto-spawn, victory detection
        ├── world_system        - Placement, spawning, game-over checks
        ├── tower_system        - Cooldowns, targeting, attack creation and resolution
        ├── enemy_system        - Movement, status effects, module ticking
        └── render_system       - Drawing map, entities, attacks, UI
```

## License
MIT — see [LICENSE](LICENSE) for details.
