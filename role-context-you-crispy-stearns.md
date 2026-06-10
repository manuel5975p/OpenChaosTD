# Container Migration Implementation Plan

## Context

OpenChaosTD currently uses standard C++ containers (`std::vector`, `std::queue`,
`std::unordered_map`) in several hot-path locations where they incur runtime heap
allocations, pointer-chasing indirection, and O(n) erase-compaction. The codebase
already has three well-designed custom containers — `DenseSlotMap`, `Grid2D`, and
`ObjectPool` — but they aren't applied to all the right places. This plan identifies
the specific hot-path sites where standard containers should be replaced with
cache-friendly alternatives, organized by priority for phased execution.

The primary goal is to eliminate runtime heap allocations from the simulation
tick/render loops and improve cache locality for high-frequency iteration.

---

## Phase 1: SmallVector — New Foundation Container

**Rationale:** Many hot-path vectors have small typical sizes (1-8 elements) but
currently pay the cost of a separate heap allocation per instance. A `SmallVector<T,N>`
stores up to N elements inline (on the stack / inside the parent object) and spills
to the heap only when exceeded. For the vast majority of use cases in this codebase,
the inline capacity is never exceeded, eliminating heap allocations entirely.

### New file: `src/engine/lib/small_vector.hpp`

```cpp
template<typename T, size_t N>
class SmallVector {
    // Inline storage: N * sizeof(T) bytes inside the object
    // Heap fallback: allocates when size > N, like std::vector
    // API mirrors std::vector: push_back, pop_back, begin/end,
    //   operator[], size, clear, reserve, erase_at (swap-pop), erase_if (stable)
    // Move is O(1): steals heap buffer or copies inline pointer
    // Copy is deep (matches std::vector semantics)
};
```

Key design points:
- **`erase_at(idx)`** — swap-and-pop (O(1)), matches `ObjectPool::ReleaseAt` pattern.
  Used where element order doesn't matter (e.g., removing effects by type).
- **`erase_if(pred)`** — stable scan-and-compact (O(n)), matches `std::erase_if`.
  Used where stable iteration order matters.
- Iterators invalidate on growth (same as `std::vector`). Pointers to elements are
  NOT stable across mutations.
- Range-for loops compile unchanged.

---

## Phase 2: Per-Entity Vector Elimination (uses SmallVector)

### 2A. `Enemy::m_effects` (`enemy.hpp:29`)

**Current:** `std::vector<Effect>` — heap allocation per enemy. Every enemy carries
0-5 effects. Iterated every frame in `EnemySystem::TickEnemies` for damage/burn/slow,
plus linear scan in `AddEffect` for refresh detection.

**Replace with:** `SmallVector<Effect, 8>`

**Impact:** Eliminates one heap allocation per enemy. Effect iteration becomes purely
inline access (single cache line). The `AddEffect` linear scan and
`std::erase_if` (line 89 of enemy_system.cpp, line 125 of enemy.hpp) become
`SmallVector::erase_if`. The `Enemy::Clone()` copy line works unchanged.

### 2B. `Enemy::m_modules` (`enemy.hpp:30`)

**Current:** `std::vector<std::unique_ptr<EnemyModule>>` — heap allocation per enemy.
1-5 modules. Iterated every frame in `TickEnemies` (line 41), `RecomputeLive` (line 39),
and `PatchStats` (line 40). Module pointers are cached (`m_baseStats`, `m_shield`) to
avoid scanning on the hottest damage/targeting paths.

**Replace with:** `SmallVector<std::unique_ptr<EnemyModule>, 8>`

**Impact:** Eliminates one heap allocation per enemy. The unique_ptr indirection
remains (modules are polymorphic and heap-allocated), but the container itself is
inline.

### 2C. `Tower::m_modules` (`tower.hpp:28`)

**Current:** `std::vector<std::unique_ptr<TowerModule>>` — same pattern as Enemy.
1-5 modules. Iterated every frame in `RecomputeStats`, `Fire`, `PatchStats`.

**Replace with:** `SmallVector<std::unique_ptr<TowerModule>, 8>`

**Impact:** Eliminates one heap allocation per tower. Same reasoning as Enemy.

### 2D. `sizeof(Enemy)` static_assert (`enemy.hpp:136-139`)

After migrating `m_effects` and `m_modules`, `sizeof(Enemy)` changes. Update
`kExpectedEnemySize` to the new value. The static_assert exists precisely to
catch layout changes like this — it is the safety net, not a risk.

---

## Phase 3: Attack Pipeline — Per-Shot Allocations (uses SmallVector)

### 3A. `AttackPayload` vectors (`attack.hpp:17,23,25,26`)

| Field | Current | Replace with |
|-------|---------|-------------|
| `m_targetKeys` | `std::vector<DenseSlotMap<Enemy>::Key>` | `SmallVector<DenseSlotMap<Enemy>::Key, 16>` |
| `m_effects` | `std::vector<Effect>` | `SmallVector<Effect, 8>` |
| `m_impactDescs` | `std::vector<EmitterDesc>` | `SmallVector<EmitterDesc, 4>` |
| `m_critImpactDescs` | `std::vector<EmitterDesc>` | `SmallVector<EmitterDesc, 4>` |

**Impact:** Each tower shot creates an Attack with up to 4 heap-allocated vectors.
During active waves (dozens of shots/second), this is significant allocation churn.
All four fields are populated once at fire time (`BuildPayload` in tower_system.cpp),
iterated once during damage resolution (`TickAttacks`), then destroyed. Typical sizes
are 1-5 targets, 1-3 effects, 1-2 impact descs — all well within inline capacity.

### 3B. `AttackVisual::m_targetPositions` (`attack.hpp:34`)

**Current:** `std::vector<Vector2>` — fire-time snapshot of target positions, iterated
for drawing every frame over the attack's visual lifetime.

**Replace with:** `SmallVector<Vector2, 16>`

### 3C. `TowerSystem::FindEnemiesInRange` return type (`tower_system.cpp:112`)

**Current:** Returns `std::vector<Enemy*>` — allocated per tower per frame.

**Replace with:** `SmallVector<Enemy*, 64>` as return type. Called only from
`FindTargets` (same file), result is immediately sorted and iterated. Move
semantics make the return cheap.

### 3D. `TowerSystem::FindTargets` local (`tower_system.cpp:94,104`)

**Current:** `std::vector<Enemy*> inRange` and `std::vector<...Key> result` —
both allocated per tower per frame.

**Replace with:** `SmallVector<Enemy*, 64>` and `SmallVector<DenseSlotMap<Enemy>::Key, 16>`.

### 3E. `TowerSystem::Fire` local (`tower_system.cpp:63`)

**Current:** `std::vector<Vector2> targetPositions` with `reserve(targetKeys.size())`.

**Replace with:** `SmallVector<Vector2, 16>`.

### 3F. Deferred-erase scratch vectors (`world_system.cpp:81,96,111`)

| Variable | Current | Replace with |
|----------|---------|-------------|
| `enemyErase` (CheckEnemyReachedCore) | `std::vector<...Key>` | `SmallVector<...Key, 64>` |
| `toRemove` (CheckEnemyDead) | `std::vector<...Key>` | `SmallVector<...Key, 128>` |
| `requests` (CheckEnemyDead) | `std::vector<SpawnRequest>` | `SmallVector<SpawnRequest, 16>` |

These are populated per frame then iterated for erasure. Already follow the deferred
erase pattern correctly. Moving to `SmallVector` eliminates their per-frame heap
allocation.

---

## Phase 4: `m_attacks` — From `std::vector` to `ObjectPool`

### File: `game_data.hpp:30`, `tower_system.cpp:56-203`, `render_system.cpp:137-155`

**Current:** `std::vector<Attack>` — `push_back` on fire (can trigger reallocation
during bursty firing), `std::erase_if` every frame for expiry (O(n) scan-and-compact).

**Replace with:** `ObjectPool<Attack>` with capacity 256. Attacks live in a pre-allocated
contiguous array. No heap allocations at runtime.

**Lifecycle change:**
- **Create:** `Attack* slot = attacks.Acquire(); if (slot) *slot = std::move(attack);`
  (Existing `Acquire()` returns a pointer to a default-constructed slot — move-assign
  into it. No ObjectPool API changes needed.)
- **Tick:** Change from range-for + `std::erase_if` to index-based `while` loop with
  `ReleaseAt(i)` for expired attacks (swap-and-pop pattern, matching ParticleSystem).
- **Draw:** Range-for still works (ObjectPool has `begin()`/`end()`).

**Signature changes:**
| File | Old | New |
|------|-----|-----|
| `tower_system.hpp:14-15` | `std::vector<Attack>&` | `ObjectPool<Attack>&` |
| `tower_system.cpp:15,56,81,180` | `std::vector<Attack>&` | `ObjectPool<Attack>&` |
| `render_system.hpp:23` | `const std::vector<Attack>&` | `const ObjectPool<Attack>&` |
| `game_data.hpp:30` | `std::vector<Attack>` | `ObjectPool<Attack>` (capacity in constructor) |
| `game_data.cpp:32,86` | `m_attacks.clear()` | `m_attacks.Clear()` |

**Attack capacity:** 256 is conservative — worst case is all ~50 towers fire
simultaneously with long-duration visuals. `Acquire()` returns `nullptr` on overflow
(silent drop, same policy as particle system).

**Render system:** `DrawAttacks` (render_system.cpp:137-155) uses range-for — works
unchanged with ObjectPool.

---

## Phase 5: Path Flattening

### File: `map.hpp:62`, `map.cpp:132-159`, all path consumers

**Current:** `std::vector<std::vector<Vector2>> m_paths` — vector of vectors.
Double-indirection: one heap allocation for the outer vector, one per inner path.
Each enemy reads `m_paths[nest][waypoint]` every frame during movement.

**Replace with:** Flat `std::vector<Vector2> m_pathPoints` + `std::vector<uint32_t>
m_pathOffsets` (start index per nest) + `std::vector<uint32_t> m_pathSizes` (or
compute size from offsets[i+1] - offsets[i]).

**Access pattern change:** `m_paths[nest][idx]` → `m_pathPoints[m_pathOffsets[nest] + idx]`

**New accessors on Map:**
```cpp
const std::vector<Vector2>& GetPathPoints() const { return m_pathPoints; }
uint32_t GetPathOffset(int nest) const { return m_pathOffsets[nest]; }
uint32_t GetPathSize(int nest) const { return m_pathSizes[nest]; }
```
Remove `const std::vector<std::vector<Vector2>>& GetPaths() const`.

**Sites to update:**
| File | Line(s) | Change |
|------|---------|--------|
| `enemy_system.cpp` | 14, 48 | `map.GetPaths()[nest][idx]` → offset-based access |
| `render_system.cpp` | 40-46 | `DrawPaths` — nested loop over offset+size |
| `world_system.cpp` | 71 | `GetPathSize(nest) - 2` |
| `world_system.cpp` | 129-131 | offset-based access for death fan-out |
| `map_serialization.cpp` | (if referencing paths) | update to new accessors |

**Impact:** Reduces `m_paths` from N+1 heap allocations (1 outer + N inner) to 2-3
heap allocations (flat point array + offset array). Improves cache locality when
multiple enemies read waypoints from the same or adjacent nests. Read pattern is
already random access into the inner vector, so the flattened access is identical
in cost.

---

## Phase 6: Pathfinder Internal Elimination

### File: `src/systems/pathfinder.cpp` (entire file rewrite of internals)

**Current problems:**
1. `Grid2D<std::vector<Edge>> m_adj` — per-cell heap-allocated edge vector (~200
   allocations for a 15×19 map)
2. `std::queue<std::pair<int, int>>` — deque-backed BFS queue (chunked heap storage)
3. `BuildGraph` — builds adjacency graph that's immediately consumed by BFS

**Solution:** Since the grid is 4-connected and walkability is known via the
`WalkableMask`, adjacency is implicit — compute neighbors on-the-fly during BFS.

**Changes:**
1. Remove `Graph` class and `BuildGraph` function entirely
2. Rewrite `BfsSolve` to check the 4 neighbor offsets against `WalkableMask` directly
3. Replace `std::queue<std::pair<int,int>>` with a stack-allocated array queue:
   `std::array<Cell, 1024>` with head/tail indices (max grid size ~500 cells)
4. `Pathfinder::Solve` API unchanged — return type is still `Grid2D<Node>`

**Impact:** Eliminates ~200+ heap allocations per pathfinding solve (every tower
placement/removal). BFS frontier becomes cache-friendly inline array. Algorithm
behavior is identical — same BFS, same output.

---

## Containers to KEEP (No Changes)

These containers are correctly used and should not be changed:

| Container | Location | Reason |
|-----------|----------|--------|
| `DenseSlotMap<Tower>` | `game_data.hpp:28` | Already optimal — stable keys + dense iteration |
| `DenseSlotMap<Enemy>` | `game_data.hpp:29` | Already optimal |
| `DenseSlotMap<Emitter>` | `particle_system.hpp:92` | Already optimal |
| `Grid2D<Tile>` | `map.hpp:57` | Already optimal |
| `Grid2D<Node>` | `map.hpp:55` | Already optimal |
| `ObjectPool<Particle>` | `particle_system.hpp:91` | Already optimal |
| `unordered_map<string, Texture2D>` | `resources.hpp:71` | Perfect use — string-keyed asset cache, cold path |
| `unordered_map<string, Sound>` | `resources.hpp:72` | Same |
| `unordered_map<string, Binding>` | `input.hpp:48` | String-keyed, read-heavy but tiny |
| `unordered_map<string, TowerTemplate>` | `tower_factory.hpp:55` | Load-time only |
| `unordered_map<string, EnemyTemplate>` | `enemy_factory.hpp:49` | Load-time only |
| `unordered_map<string, Enemy>` | `wave_manager.hpp:120-121` | Wave start only |
| `vector` in profiler, text_renderer, settings, editors | various | Cold path or trivially small, no performance impact |

---

## Verification Strategy

### Build verification
- Compile with `-Wall -Wextra -Werror` after each phase.
- The `sizeof(Enemy)` static_assert will fire after Phase 2 — update the constant.

### Functional verification
1. **Phase 2-3 (SmallVector):** Run the game, build towers, spawn waves. Verify:
   - Tower targeting and firing (visual attack lines appear)
   - Status effects (burn particles, slow, stun)
   - Enemy death and split spawning
   - Enemy Clone (used in wave preview and spawn)
2. **Phase 4 (ObjectPool<Attack>):** Verify attacks render for their full visual
   duration. Test with many towers firing simultaneously. Verify no attack pool
   overflow in normal play.
3. **Phase 5 (Path flattening):** Enable debug path rendering. Place/remove towers
   and verify paths rebuild correctly. Verify enemies follow paths correctly.
4. **Phase 6 (Pathfinder):** Compare `DebugDrawMap` output (distance values and flow
   arrows) before and after. Must be byte-identical since the BFS algorithm is unchanged.

### Performance verification
- Use the built-in `Profiler` to measure frame times before and after each phase.
- Key metrics: `TickEnemies`, `TowerSystem::Update`, `TickAttacks`, `FollowPath`.

---

## Execution Order Summary

| Phase | What | New Container? | Files Changed |
|-------|------|---------------|---------------|
| 1 | Write `SmallVector<T,N>` | **Yes** (`small_vector.hpp`) | 1 new file |
| 2 | Per-entity vectors → SmallVector | No | `enemy.hpp`, `tower.hpp` |
| 3 | Attack pipeline + scratch vectors → SmallVector | No | `attack.hpp`, `tower_system.cpp/.hpp`, `world_system.cpp` |
| 4 | `m_attacks` → ObjectPool | No (extends usage) | `game_data.hpp/.cpp`, `tower_system.cpp/.hpp`, `render_system.cpp/.hpp` |
| 5 | Path flattening | No | `map.hpp/.cpp`, `enemy_system.cpp`, `render_system.cpp`, `world_system.cpp` |
| 6 | Pathfinder internal elimination | No | `pathfinder.cpp` |

Each phase is independently testable and can be committed as a separate changeset.
Phases 2-3 depend on Phase 1 (SmallVector). Phases 4-6 are independent and can be
done in any order after Phase 1.
