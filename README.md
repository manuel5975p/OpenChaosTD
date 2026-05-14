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
| [raylib](https://github.com/raysan5/raylib) | 5.5 | Window, rendering, input, audio |
| [nlohmann/json](https://github.com/nlohmann/json) | 3.11.3 | JSON parsing for data files |

## Media
*(Add a screenshot or GIF here)*

## Project Structure
```
OpenChaosTD/
├── assets/
│   └── textures/               - Sprites for towers, enemies and tiles
│
├── data/
│   ├── config.json             - Window and display settings
│   ├── gameplay.json           - Starting lives and gold
│   ├── keybindings.json        - Input action bindings (rebindable)
│   ├── towers.json             - Tower type definitions (stats, modules)
│   └── enemies.json            - Enemy type definitions (stats, modules)
│
└── src/
    ├── main.cpp                - Entry point
    ├── game.hpp/.cpp           - Game loop, state machine, manager accessors
    │
    ├── core/                   Engine infrastructure — see core/CORE.md
    │   ├── asset_manager       - Load/cache textures, sounds, fonts, music
    │   ├── renderer            - Virtual resolution + letterbox scaling
    │   ├── input_manager       - Action-based input with JSON keybindings
    │   ├── game_config         - Window/display settings loaded from JSON
    │   ├── jsonio              - Cross-platform JSON read/write (desktop + web)
    │   ├── performance_monitor - Frame-time profiling (avg, last, peak)
    │   └── button              - Shared UI primitive (hit-test + draw)
    │
    ├── factory/                Data-driven entity construction from JSON
    │   ├── tower_factory       - Builds Tower instances from towers.json
    │   └── enemy_factory       - Builds Enemy instances from enemies.json
    │
    ├── hud/                    In-game HUD elements
    │   └── tower_hud           - Tower selection panel with cost display
    │
    ├── lib/                    Reusable data structures
    │   ├── dense_slotmap.hpp   - Stable-ID container optimised for iteration
    │   ├── slotmap.hpp         - Stable-ID container optimised for lookup
    │   └── grid2d.hpp          - Resizable 2D array template
    │
    ├── states/                 Game screen state machine
    │   ├── game_state.hpp      - Abstract base (ProcessInput, Update, Draw)
    │   ├── menu_state          - Main menu
    │   ├── play_state          - Active gameplay
    │   └── end_state           - Victory / game over screen
    │
    ├── world/                  Entity definitions and data
    │   ├── game_data           - Runtime world state + starting values (JSON)
    │   ├── tower               - Tower entity (position, stats, modules)
    │   ├── tower_modules.hpp   - FlatDamageModule, SlowModule
    │   ├── enemy               - Enemy entity (position, health, effects, modules)
    │   ├── enemy_module.hpp    - RegenerationModule, ArmorModule, ResistanceModule
    │   ├── attack.hpp          - Attack object (origin, targets, payload, visual)
    │   ├── effect.hpp          - Status effect (Burn, Slow) with duration
    │   ├── map                 - Grid, nest/core placement, path construction
    │   └── tile.hpp            - Tile type, walkable/buildable flags
    │
    └── systems/                Per-frame game logic
        ├── pathfinder.hpp      - BFS pathfinding from core to all nests
        ├── world_system        - Placement, spawning, game-over checks
        ├── tower_system        - Cooldowns, targeting, attack creation and resolution
        ├── enemy_system        - Movement, status effects, module ticking
        └── render_system       - Drawing map, entities, attacks, UI
```

## License
MIT — see [LICENSE](LICENSE) for details.
