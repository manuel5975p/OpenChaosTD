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
│   ├── data/
│   │   ├── towers.json         - Tower type definitions (stats, modules)
│   │   └── enemies.json        - Enemy type definitions (stats, modules)
│   └── textures/               - Sprites for towers, enemies and tiles
│
└── src/
    ├── main.cpp                - Entry point
    ├── game.hpp/.cpp           - Window, game loop, state machine, factories
    │
    ├── core/                   Engine infrastructure
    │   ├── asset_manager       - Load/cache textures, sounds, fonts, music
    │   ├── renderer            - Rendering with letterbox scaling
    │   ├── input_manager       - Action-based input and keybindings
    │   ├── jsonio              - Cross-platform JSON read/write (desktop + web)
    │   └── performance_monitor - Frame-time profiling (avg, last, peak)
    │
    ├── factory/                Data-driven entity construction from JSON
    │   ├── tower_factory       - Builds Tower instances from towers.json
    │   └── enemy_factory       - Builds Enemy instances from enemies.json
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
    │   └── game_over_state     - Game over screen
    │
    ├── world/                  Entity definitions and data
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
        ├── world_system        - Spawning, placement, damage, cleanup
        ├── tower_system        - Cooldowns, targeting, attack creation
        ├── enemy_system        - Movement, status effects, module ticking
        └── render_system       - Drawing map, entities, attacks, UI
```

## License
MIT — see [LICENSE](LICENSE) for details.
