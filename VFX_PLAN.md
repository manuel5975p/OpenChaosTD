# VFX Implementation Plan — Tower-Level Visual Descriptor

## Approach

Attacks currently conflate damage logic and visual state in one flat struct.
This plan separates them cleanly, adds a particle system, and gives each tower
a fully defined visual identity that lives in the factory — modules own gameplay,
towers own looks.

Implementation order is bottom-up: each phase is independently compilable and
testable before the next one starts.

---

## Phase 1 — Particle System

The foundation. No knowledge of attacks, towers, or effects — just owns, ticks,
and draws live particles.

### New files

**`src/world/particle.hpp`**

Two structs:

`EmitterDesc` — describes how to spawn a burst. Stored wherever emission is
triggered (Tower, Effect, death site). Never changes after creation.

```cpp
struct EmitterDesc {
    Color color          = WHITE;
    Color endColor       = {255, 255, 255, 0}; // lerp target over lifetime
    int   count          = 0;                  // 0 = disabled
    float speed          = 50.0f;
    float speedVariance  = 20.0f;
    float spread         = 3.14159f;           // half-angle; PI = full circle
    float angle          = 0.0f;               // base direction in radians
    float lifetime       = 0.2f;
    float lifetimeVariance = 0.05f;
    float size           = 3.0f;
    float endSize        = 0.0f;               // shrinks to this
};
```

`Particle` — one live particle. Only lives inside ParticleSystem.

```cpp
struct Particle {
    Vector2 position;
    Vector2 velocity;
    float   lifetime;
    float   maxLifetime;
    Color   color;
    Color   endColor;
    float   size;
    float   endSize;
};
```

---

**`src/systems/particle_system.hpp`**

```cpp
class ParticleSystem {
public:
    // Spawn count particles at origin; baseVelocity added to each (use enemy
    // velocity for effect particles so they don't lag behind moving targets).
    void Emit(Vector2 origin, const EmitterDesc& desc,
              Vector2 baseVelocity = {0, 0});
    void Tick(float dt);
    void Draw() const;
    void Clear();
private:
    std::vector<Particle> m_particles;
};
```

---

**`src/systems/particle_system.cpp`**

`Emit`:
- Loop `desc.count` times.
- Pick a random angle in `[desc.angle - desc.spread, desc.angle + desc.spread]`.
- Randomise speed in `[desc.speed - desc.speedVariance, desc.speed + desc.speedVariance]`,
  clamped to >= 0.
- velocity = direction * speed + baseVelocity.
- Randomise lifetime in `[desc.lifetime - desc.lifetimeVariance, desc.lifetime + desc.lifetimeVariance]`.
- Push a `Particle` with all fields set; `maxLifetime = lifetime`.

`Tick`:
- For each particle: `position += velocity * dt`.
- Apply drag: `velocity *= powf(0.08f, dt)` (strong drag, particles decelerate quickly).
- `lifetime -= dt`.
- `std::erase_if` particles with `lifetime <= 0`.

`Draw`:
- For each particle: `float t = lifetime / maxLifetime` (0=dead, 1=fresh).
- `Color c = ColorLerp(particle.endColor, particle.color, t)`.
- `float s = Lerp(particle.endSize, particle.size, t)`.
- `DrawCircleV(particle.position, s, c)`.

`Clear`: clears `m_particles`.

---

### Changes to existing files

**`src/world/game_data.hpp`**
- Add `#include <systems/particle_system.hpp>`.
- Add `ParticleSystem m_particles;` member.

**`src/world/game_data.cpp`** (`Reset`)
- Call `m_particles.Clear()`.

**`src/states/play_state.cpp`** (`Update`)
- After `m_towerSystem.TickAttacks(...)`, call `gameData.m_particles.Tick(dt)`.

**`src/states/play_state.cpp`** (`Draw`, inside `BeginMode2D`)
- After `DrawAttacks(...)`, call `gameData.m_particles.Draw()`.

### Verification

Temporarily emit a burst on a keypress (e.g. debug key) to confirm particles
spawn, move, fade, and are cleaned up. Remove the debug call before Phase 2.

---

## Phase 2 — Separate Attack Visual and Logic

Purely structural: split the single `Attack` struct into `AttackPayload`
(damage) and `VfxEffect` (visual). Behaviour stays identical — no particles
are wired up yet. The game should compile and look unchanged after this phase.

### Changed file: `src/world/attack.hpp`

Rename `Attack` to `AttackPayload`. Remove all visual fields
(`m_origin`, `m_targetPositions`, `m_type` for rendering, `m_duration`,
`m_maxDuration`, `Progress()`). Keep only:

```cpp
struct AttackPayload {
    std::vector<DenseSlotMap<Enemy>::Key> m_targetKeys;
    AttackType m_targetShape;  // Area or Line — targeting only, not rendering
    float m_radius;            // used by Area for the in-range check

    float m_damage         = 0.0f;
    float m_armorPierce    = 0.0f;
    float m_critChance     = 0.0f;
    float m_critMultiplier = 1.0f;
    bool  m_wasCrit        = false; // set by TickPayloads when crit fires
    std::vector<Effect> m_effects;

    float m_delay    = 0.0f;
    bool  m_resolved = false;
    float m_ttl      = 0.0f;  // expires after resolution; set = attackDuration
};
```

`AttackPayload` has no duration-based `Progress()` — it just needs a TTL so it
can be erased after resolution settles. Set `m_ttl = attackDuration` when
creating it; `TickPayloads` decrements it and erases on <= 0.

---

### New file: `src/world/vfx_effect.hpp`

```cpp
#pragma once
#include <raylib.h>
#include <vector>
#include <world/particle.hpp>

enum class VfxStyle { Beam, Burst, Ring, Zap };

struct VfxEffect {
    Vector2              m_origin;
    std::vector<Vector2> m_targetPositions;
    VfxStyle m_style        = VfxStyle::Beam;
    Color    m_color        = {255, 220, 50, 255};
    float    m_radius       = 0.0f;    // for Ring: max radius of the expanding ring
    float    m_duration     = 0.0f;
    float    m_maxDuration  = 0.0f;

    // Trail emitter — filled in Phase 4; ignored until then
    EmitterDesc m_trailDesc;
    float       m_trailRate        = 0.0f; // particles/sec; 0 = no trail
    float       m_trailAccumulator = 0.0f;

    float Progress() const {
        return (m_maxDuration > 0.0f) ? m_duration / m_maxDuration : 0.0f;
    }
};
```

---

### Changed file: `src/world/game_data.hpp`

Replace:
```cpp
std::vector<Attack> attacks;
```
With:
```cpp
std::vector<AttackPayload> m_payloads;
std::vector<VfxEffect>     m_vfx;
```

All callers that used `gameData.attacks` must be updated (tower_system, render_system,
play_state).

---

### Changed file: `src/systems/tower_system.cpp`

**`update()`**: when an attack fires, create one of each:

```cpp
AttackPayload payload;
payload.m_targetKeys  = tower.m_currentTargetKeys;
payload.m_targetShape = tower.m_stats.attackType;
payload.m_radius      = tower.m_stats.radius;
payload.m_ttl         = tower.m_stats.attackDuration;
payload.m_delay       = 0.0f;
BuildPayload(tower, payload);           // mods call Contribute(payload)

VfxEffect vfx;
vfx.m_origin          = tower.m_position;
vfx.m_targetPositions = targetPositions;
vfx.m_duration        = tower.m_stats.attackDuration;
vfx.m_maxDuration     = tower.m_stats.attackDuration;
vfx.m_radius          = tower.m_stats.radius;
vfx.m_style           = VfxStyle::Beam;       // placeholder until Phase 4
vfx.m_color           = {255, 220, 50, 255};  // placeholder

gameData.m_payloads.push_back(std::move(payload));
gameData.m_vfx.push_back(std::move(vfx));
```

`BuildPayload` replaces `BuildAttackPayload` — same logic, parameter type updated.
The module `Contribute` signature changes from `Attack&` to `AttackPayload&`.

**`TickAttacks` splits into two methods:**

`TickPayloads(float dt, GameData&)`:
- Tick `m_ttl -= dt` for each payload.
- After `m_delay` elapses and `!m_resolved`: apply damage (existing armor/crit/intercept
  logic unchanged), set `m_resolved = true`.
- `std::erase_if` where `m_ttl <= 0`.

`TickVfx(float dt, GameData&)`:
- Tick `m_duration -= dt` for each effect.
- (Trail emission added in Phase 3.)
- `std::erase_if` where `m_duration <= 0`.

**`tower_system.hpp`**: replace `TickAttacks` with `TickPayloads` and `TickVfx`.

---

### Changed file: `src/systems/render_system.cpp`

Rename `DrawAttacks` → `DrawVfx`, parameter changes from `const std::vector<Attack>&`
to `const std::vector<VfxEffect>&`.

Body is updated for the enum — for now use the same Area/Line logic but reading
`vfx.m_style` instead of `attack.m_type`, and `vfx.m_color` instead of hardcoded
yellow. This is the placeholder that Phase 3 replaces with per-style rendering.

---

### Changed file: `src/states/play_state.cpp`

- `Update`: replace `TickAttacks` with `TickPayloads` then `TickVfx`.
- `Draw`: replace `DrawAttacks(gameData.attacks)` with `DrawVfx(gameData.m_vfx)`.

### Verification

Run the game. Visually nothing should change — attacks still look like yellow
lines/circles, damage behaviour is identical. Confirm no crashes, no memory leaks,
no dropped attacks.

---

## Phase 3 — Wire Up Particle Emission

All five emission sources are connected. At the end of this phase every attack
type spawns particles; burning/slowing enemies are visually distinct; enemies
burst on death.

Placeholder `EmitterDesc` values are used throughout — they are replaced with
final values in Phase 4.

---

### 3a — Muzzle particles

Emitted once, immediately when the tower fires, in `TowerSystem::update`:

```cpp
// After building payload and vfx, before pushing them:
gameData.m_particles.Emit(tower.m_position, tower.m_vfx.muzzleDesc);
```

`tower.m_vfx` does not exist until Phase 4. Use a generic placeholder for now:

```cpp
EmitterDesc muzzlePlaceholder;
muzzlePlaceholder.count    = 6;
muzzlePlaceholder.color    = {255, 240, 100, 255};
muzzlePlaceholder.endColor = {255, 200, 50, 0};
muzzlePlaceholder.speed    = 60.0f; muzzlePlaceholder.speedVariance = 20.0f;
muzzlePlaceholder.spread   = 0.8f;
muzzlePlaceholder.lifetime = 0.1f;
muzzlePlaceholder.size     = 3.0f;
gameData.m_particles.Emit(tower.m_position, muzzlePlaceholder);
```

Remove this placeholder in Phase 4 when `tower.m_vfx.muzzleDesc` is live.

---

### 3b — Impact particles

`AttackPayload` gains two new fields (add to the struct in `attack.hpp`):

```cpp
EmitterDesc m_impactDesc;
EmitterDesc m_critImpactDesc; // used on crit if count > 0, else m_impactDesc
```

In `TowerSystem::TickPayloads`, when `m_resolved` flips:

```cpp
payload.m_wasCrit = /* crit rolled in this resolve pass */;

bool useCrit = payload.m_wasCrit && payload.m_critImpactDesc.count > 0;
const EmitterDesc& desc = useCrit ? payload.m_critImpactDesc : payload.m_impactDesc;

for (auto& key : payload.m_targetKeys) {
    if (Enemy* e = gameData.enemies.Get(key))
        gameData.m_particles.Emit(e->m_position, desc);
}
```

Move the crit roll earlier in the resolve block so `m_wasCrit` is set before
the emit call.

Populate `m_impactDesc` in `TowerSystem::update` with the placeholder; replace
in Phase 4 from `tower.m_vfx`.

---

### 3c — Trail particles

`TowerSystem::TickVfx` emits trail particles for effects with `m_trailRate > 0`:

```cpp
if (vfx.m_trailRate > 0.0f) {
    vfx.m_trailAccumulator += vfx.m_trailRate * dt;
    while (vfx.m_trailAccumulator >= 1.0f) {
        gameData.m_particles.Emit(SampleVfxPoint(vfx), vfx.m_trailDesc);
        vfx.m_trailAccumulator -= 1.0f;
    }
}
```

`SampleVfxPoint` (static helper in tower_system.cpp):
- For `Beam` / `Zap`: pick a random `t` in [0,1], lerp between `m_origin` and a
  random target position.
- For `Ring`: pick a random angle, return a point on a circle of radius
  `(1.0f - vfx.Progress()) * vfx.m_radius` centred on `m_origin`.
- For `Burst`: return a random point within the burst radius around the target.

Trail rate stays 0 for most placeholder VfxEffects; set per-tower in Phase 4.

---

### 3d — Per-style rendering in DrawVfx

Replace the placeholder switch in `render_system.cpp` with distinct rendering
per `VfxStyle`:

```
Beam:
  Two overlapping lines per target — a wide soft outer line (width 4, alpha*0.4)
  and a thin bright inner line (width 2, alpha*0.9). Both use vfx.m_color.
  The double layer gives the beam perceived glow without a shader.

Zap:
  Single thin line (width 1.5) per target, full alpha at start, instant fade.
  High alpha early in Progress() then drops sharply. Consider drawing a second
  slightly offset line (+1px) in a darker shade for a jagged feel.

Burst:
  No line drawn. A small circle at each target position that expands as it fades:
  radius = (1.0f - Progress()) * 18.0f
  Fill alpha = Progress() * 200, outline alpha = Progress() * 255.
  Gives the impression of an explosion that blooms outward.

Ring:
  Expanding circle outline at m_origin.
  radius = (1.0f - Progress()) * m_radius   (starts small, expands to full range)
  A faint fill (alpha * 0.12) plus a solid outline (alpha * 0.9).
  Used by area towers (freezer, flamer).
```

---

### 3e — Active effect particles (Burn / Slow)

`Effect` gains three fields (in `src/world/effect.hpp`):

```cpp
EmitterDesc m_particleDesc;
float m_emitRate        = 0.0f;  // particles/sec; 0 = no visual
float m_emitAccumulator = 0.0f;
```

`EnemySystem::TickEnemies` already iterates every active effect per enemy.
Add the emission block at the top of the effect loop, before Burn/Slow logic:

```cpp
if (effect.m_emitRate > 0.0f) {
    effect.m_emitAccumulator += effect.m_emitRate * dt;
    while (effect.m_emitAccumulator >= 1.0f) {
        // Pass enemy velocity as base so particles don't lag on fast enemies
        gameData.m_particles.Emit(enemy.m_position, effect.m_particleDesc,
                                  /* enemy velocity estimate: */ {0, 0});
        effect.m_emitAccumulator -= 1.0f;
    }
}
```

`TickEnemies` already takes `GameData&` so `gameData.m_particles` is accessible.

The `m_particleDesc` and `m_emitRate` are set when the Effect is constructed.
Since Effects are created inside module `Contribute()` calls, the module itself
stores the descriptor and copies it onto the Effect it pushes:

`BurnModule` gains:
```cpp
EmitterDesc m_particleDesc;  // set in constructor
float       m_emitRate;      // set in constructor
```

`BurnModule::Contribute`:
```cpp
Effect e(EffectType::Burn, m_duration, m_value);
e.m_particleDesc = m_particleDesc;
e.m_emitRate     = m_emitRate;
attack.m_effects.push_back(std::move(e));
```

Same pattern for `SlowModule`. Default values for Phase 3 (replaced in Phase 4):

- Burn: orange/red particles, count=1 per emit, upward angle (PI*1.5), spread=0.6,
  speed=25, lifetime=0.3, size=3. Rate = 18/sec.
- Slow: pale cyan particles, count=1 per emit, any direction, speed=12, lifetime=0.4,
  size=2. Rate = 8/sec.

---

### 3f — Death particles

In `WorldSystem::CheckEnemyDead`, just before `RemoveEnemy(key, gameData)`:

```cpp
EmitterDesc deathDesc;
deathDesc.color        = {220, 55, 55, 255};
deathDesc.endColor     = {180, 30, 30, 0};
deathDesc.count        = 14;
deathDesc.speed        = 90.0f; deathDesc.speedVariance = 30.0f;
deathDesc.spread       = 3.14159f;
deathDesc.lifetime     = 0.4f;
deathDesc.size         = 4.5f;  deathDesc.endSize = 0.0f;
gameData.m_particles.Emit(enemy->m_position, deathDesc);
```

`CheckEnemyDead` already takes `GameData&` so this is a one-liner addition.
A per-enemy-type descriptor can be added later by storing an `EmitterDesc` on
`Enemy` (same pattern as `TowerVfxDesc` below).

### Verification

Each tower should produce muzzle sparks on fire and impact sparks on hit.
Burning enemies flicker orange. Slowed enemies drift cyan. Dying enemies burst.
Beam/Ring/Burst/Zap attacks each look visually distinct.

---

## Phase 4 — Define Visual Identity Per Tower

Replace all placeholder descs and style values with fully specified per-tower
definitions. This is the phase where each tower gets its own personality.

### New struct: `TowerVfxDesc`

Add to `src/world/tower.hpp` (or a new `src/world/tower_vfx.hpp` if preferred):

```cpp
struct TowerVfxDesc {
    VfxStyle style     = VfxStyle::Beam;
    Color    color     = {255, 220, 50, 255};

    EmitterDesc muzzleDesc;

    EmitterDesc trailDesc;
    float       trailRate = 0.0f;         // particles/sec; 0 = no trail

    EmitterDesc impactDesc;
    EmitterDesc critImpactDesc;           // used on crit if count > 0
};
```

### Tower gains `m_vfx`

In `src/world/tower.hpp`:
```cpp
TowerVfxDesc m_vfx;  // set by factory, never modified at runtime
```

### Wire `m_vfx` into TowerSystem

In `TowerSystem::update`, replace placeholders:
```cpp
// Muzzle
gameData.m_particles.Emit(tower.m_position, tower.m_vfx.muzzleDesc);

// Payload impact descs
payload.m_impactDesc     = tower.m_vfx.impactDesc;
payload.m_critImpactDesc = tower.m_vfx.critImpactDesc;

// VfxEffect visual fields
vfx.m_style     = tower.m_vfx.style;
vfx.m_color     = tower.m_vfx.color;
vfx.m_trailDesc = tower.m_vfx.trailDesc;
vfx.m_trailRate = tower.m_vfx.trailRate;
```

### TowerFactory: per-tower `m_vfx` definitions

In `TowerFactory::Create`, after setting `m_base`, populate `tower.m_vfx`.
Specified per tower below. All colors are RGBA.

---

**zapper** — fast electric arc, targets swarms

```
style:   Zap
color:   {120, 200, 255, 255}  electric blue

muzzle:  count=8, color={180,230,255}, speed=70/var=25, spread=1.0, lifetime=0.07, size=2.5
trail:   none (attack too short to need one)
impact:  count=5, color={100,190,255}, speed=55/var=20, spread=PI, lifetime=0.12, size=2
crit:    n/a
```

---

**freezer** — slow expanding frost ring

```
style:   Ring
color:   {80, 190, 255, 255}   ice cyan

muzzle:  count=6, color={200,235,255}, angle=PI*1.5 (upward), spread=0.7,
         speed=20/var=8, lifetime=0.3, size=2.5, endSize=0
trail:   rate=25/sec
         desc: count=1, color={160,220,255,180}, speed=10/var=5, spread=PI,
               lifetime=0.35, size=2, endSize=0
impact:  count=8, color={80,190,255}, speed=35/var=12, spread=PI, lifetime=0.28, size=3
crit:    n/a
```

---

**sniper** — single heavy burst, high impact

```
style:   Burst
color:   {240, 240, 255, 255}  near white

muzzle:  count=4, color={255,255,255}, angle=0 (forward, set dynamically if
         possible, else use 0), spread=0.3, speed=120/var=40, lifetime=0.05, size=3
trail:   none
impact:  count=16, color={220,220,255}, endColor={180,180,200,0},
         speed=100/var=35, spread=PI, lifetime=0.35, size=4, endSize=0
crit:    count=26, color={255,255,200}, speed=130/var=45, spread=PI,
         lifetime=0.5, size=5.5, endSize=0
```

---

**flamer** — short-range fire ring

```
style:   Ring
color:   {255, 100, 20, 255}   orange-red

muzzle:  count=10, color={255,160,40}, angle=PI*1.5 (upward), spread=1.2,
         speed=30/var=12, lifetime=0.2, size=3, endSize=0
trail:   rate=35/sec
         desc: count=1, color={255,120,30,200}, endColor={200,50,0,0},
               speed=18/var=8, spread=PI, lifetime=0.2, size=2.5, endSize=0
impact:  count=8, color={255,100,20}, speed=50/var=18, spread=PI,
         lifetime=0.22, size=3.5, endSize=0
crit:    n/a
```

---

**piercer** — tight metal beam, armor shred

```
style:   Beam
color:   {210, 220, 230, 255}  silver

muzzle:  count=6, color={230,235,240}, spread=0.25 (tight forward cone),
         speed=110/var=40, lifetime=0.06, size=3
trail:   none
impact:  count=10, color={200,210,220}, endColor={160,170,180,0},
         speed=80/var=30, spread=0.9 (narrow ricochet cone), lifetime=0.15, size=2.5
crit:    n/a (no crit module)
```

---

**gambler** — erratic shots with spectacular crits

```
style:   Zap
color:   {255, 220, 50, 255}   yellow-gold

muzzle:  count=5, color={255,230,80}, speed=60/var=25, spread=0.9, lifetime=0.1, size=2.5
trail:   none
impact:  count=6, color={255,210,40}, speed=60/var=20, spread=PI, lifetime=0.15, size=3
crit:    count=22, color={255,200,0}, endColor={220,150,0,0},
         speed=85/var=30, spread=PI, lifetime=0.45, size=5, endSize=0
         -- visually a large gold explosion, unmistakably different from a normal hit
```

---

### Effect particle descs in modules

`BurnModule` constructor sets (replacing Phase 3 placeholder):
```
particleDesc: color={255,110,30}, endColor={180,40,0,0}, count=1,
              angle=PI*1.5, spread=0.5, speed=22/var=8, lifetime=0.28, size=2.5, endSize=0
emitRate: 18
```

`SlowModule` constructor sets:
```
particleDesc: color={160,220,255,180}, endColor={80,160,220,0}, count=1,
              spread=PI, speed=10/var=4, lifetime=0.38, size=2, endSize=0
emitRate: 8
```

### Verification

Each tower has a unique visual identity. Zapper zaps in blue. Freezer pulses
cyan rings with ice drift. Sniper fires a white flash and explodes on hit, with
gold explosions on crits. Flamer sprays orange rings and fire. Piercer shoots a
silver beam with ricochet sparks. Gambler normal hits look modest; crits are
unmistakably dramatic.

Burning enemies flicker orange. Slowed enemies trail pale blue wisps. Every
dead enemy bursts. The game communicates all active states through particles
without any additional UI.

---

## File Change Summary

| File | Change |
|---|---|
| `src/world/particle.hpp` | **new** — `Particle`, `EmitterDesc` |
| `src/systems/particle_system.hpp/.cpp` | **new** — `ParticleSystem` |
| `src/world/attack.hpp` | refactor `Attack` → `AttackPayload`, add impact descs |
| `src/world/vfx_effect.hpp` | **new** — `VfxStyle`, `VfxEffect` |
| `src/world/tower.hpp` | add `TowerVfxDesc m_vfx` |
| `src/world/effect.hpp` | add `EmitterDesc`, `m_emitRate`, `m_emitAccumulator` |
| `src/world/game_data.hpp` | replace `attacks` with `m_payloads`, `m_vfx`; add `m_particles` |
| `src/world/tower_modules.hpp/.cpp` | `BurnModule`/`SlowModule` gain emitter fields |
| `src/systems/tower_system.hpp/.cpp` | split `TickAttacks` → `TickPayloads` + `TickVfx`; wire `m_vfx` |
| `src/systems/render_system.hpp/.cpp` | `DrawAttacks` → `DrawVfx` with per-style rendering |
| `src/systems/enemy_system.cpp` | effect particle emission in `TickEnemies` |
| `src/systems/world_system.cpp` | death particle emission in `CheckEnemyDead` |
| `src/factory/tower_factory.cpp` | populate `tower.m_vfx` per tower name |
| `src/states/play_state.cpp` | updated call sites for all renamed/split methods |
