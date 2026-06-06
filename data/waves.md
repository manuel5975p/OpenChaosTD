# Wave JSON Schema

Waves are not authored individually. `data/waves.json` configures a **procedural, budget-based
generator**: every wave is composed at runtime from a threat budget that grows with the wave number,
drawing enemies semi-randomly from a configurable pool. The same file also defines the win condition,
periodic boss waves, and how fast enemies scale.

There is no `"waves"` array — the four top-level keys below drive everything.

## Top-level fields

| Field              | Type   | Required | Description |
|--------------------|--------|----------|-------------|
| `budget`           | object | yes      | Threat-budget scaling and the win condition — see below |
| `boss`             | object | no       | Periodic boss-wave rules — see below |
| `upgrade_interval` | int    | no       | Enemies gain an upgrade tier every this many waves (0 = never) |
| `enemy_pool`       | array  | yes      | The enemy types the generator may draw from — see below |

All enemy names referenced anywhere in this file must match a `name` defined in `data/enemies.json`
(`data/enemies.md`).

---

## `budget` object

```json
"budget": { "type": "exponential", "base_budget": 20, "growth_rate": 0.15, "victory_wave": 0 }
```

Sets how much "threat" each wave is allowed to spend and when the game ends. Each enemy drawn into the
wave costs its pool `cost`; spawning continues until the budget can no longer afford the cheapest
available enemy. Because enemy costs stay constant, a bigger budget makes a wave denser rather than
just swapping in pricier units.

The `type` field selects one of two scaling models (defaulting to `exponential` if omitted):

**Exponential** — smooth compounding growth:

```
budget = base_budget * (1 + growth_rate) ^ (wave_number - 1)
```

**Step-Linear** — linear growth with a dip on every upgrade tier (so a tier that makes enemies tougher
also sends fewer of them). `tier` is the current upgrade tier (see `upgrade_interval`). The result is
clamped so the tier subtraction can never push a wave below `base_budget`:

```
budget = max(base_budget, base_budget + linear_step * wave_number - tier_adjustment * tier)
```

| Field             | Type   | Used by      | Description |
|-------------------|--------|--------------|-------------|
| `type`            | string | both         | `"exponential"` (default) or `"step_linear"` |
| `base_budget`     | float  | both         | Threat budget floor; the multiplier (exponential) / starting value (step-linear) |
| `growth_rate`     | float  | exponential  | Per-wave growth fraction (`0.15` = +15% each wave); default `0.15` |
| `linear_step`     | float  | step_linear  | Threat budget added per wave; default `10` |
| `tier_adjustment` | float  | step_linear  | Threat budget removed per upgrade tier; default `0` |
| `victory_wave`    | int    | both         | Wave that, once cleared, wins the game. `0` = **endless** (waves never stop) |

Fields that don't apply to the selected `type` are ignored, and any omitted field falls back to its
default, so a minimal `{ "base_budget": 20 }` block still loads (as exponential).

---

## `boss` object

```json
"boss": { "interval": 10, "boss_enemies": ["sovereign"] }
```

Every `interval` waves (wave 10, 20, 30, … for `interval: 10`) is a **boss wave**: the generator
forces exactly one enemy chosen at random from `boss_enemies`, deducts its `cost` from the budget,
and spends whatever remains on a normal escort selection. Omit the whole `boss` object (or set
`interval` to 0) to disable boss waves entirely.

| Field          | Type           | Description |
|----------------|----------------|-------------|
| `interval`     | int            | A boss wave occurs every Nth wave; `0` disables boss waves |
| `boss_enemies` | array<string\> | Pool of enemy names eligible to be the forced boss; one is picked per boss wave |

Boss enemies are **excluded from the regular escort selection** by name, so they only ever appear as
the forced boss. List each boss in `enemy_pool` as well (see below) so the generator can read its
`cost` and spawn `interval`; an unlisted boss still spawns but costs nothing and uses a default
spawn interval.

---

## `upgrade_interval`

```json
"upgrade_interval": 5
```

Tracks an **upgrade tier** that increments every `upgrade_interval` waves:

```
tier = wave_number / upgrade_interval   (integer division; 0 disables upgrades)
```

With `upgrade_interval: 5`, waves 1–4 are tier 0 (base stats), waves 5–9 are tier 1, waves 10–14 are
tier 2, and so on. Every enemy spawned in a wave has its `upgrades` from `data/enemies.json`
(`data/enemies.md`) applied `tier` times before it enters the map.

If an enemy defines fewer upgrade levels than the current tier, the generator walks the defined
levels in order and then keeps re-applying the **last** one — so multiplicative upgrades (e.g.
`"mul": { "maxHealth": 1.2 }`) compound and scaling continues indefinitely in endless mode. An
enemy with no `upgrades` is simply left at its base stats. Upgrades change stats only; the pool
`cost` is unaffected, so wave density grows purely from the budget.

---

## `enemy_pool` array

The set of enemy types the generator can place in a wave. Each entry references one enemy by name
and attaches the metadata the generator needs.

```json
"enemy_pool": [
    { "enemy": "shade",   "cost": 3,  "min_wave": 1, "interval": 1.0 },
    { "enemy": "flicker", "cost": 4,  "min_wave": 2, "interval": 0.5 },
    { "enemy": "titan",   "cost": 14, "min_wave": 6, "interval": 1.6 }
]
```

| Field      | Type   | Required | Description |
|------------|--------|----------|-------------|
| `enemy`    | string | yes      | Enemy `name` from `data/enemies.json` |
| `cost`     | int    | yes      | Threat cost deducted from the wave budget each time this enemy is drawn |
| `min_wave` | int    | yes      | Earliest wave this enemy may appear in; gates it out of earlier waves |
| `interval` | float  | yes      | Seconds between consecutive spawns of this enemy within a wave |

Use a high `min_wave` (e.g. `999`) for boss-only enemies so they never leak into regular waves while
still providing a `cost`/`interval` for boss handling.

---

## Wave generation

Each wave is composed independently from its budget:

1. **Budget** — computed from the `budget` block's scaling model (exponential or step-linear; see above).
2. **Filter** — keep pool entries whose `min_wave <= wave_number` and whose name is **not** in
   `boss_enemies`. The cheapest remaining `cost` becomes the spending floor.
3. **Boss** (boss waves only) — force one boss from `boss_enemies`, subtract its `cost`.
4. **Selection** — repeatedly pick an affordable entry at random and deduct its `cost`, stopping the
   instant the remaining budget drops below the cheapest available `cost`.
5. **Grouping** — identical picks are tallied into one spawn group per enemy type, each spawning on
   its own pool `interval`. Groups have no start delay and are distributed across all map nests
   round-robin.

### Lookahead

The generator keeps the **next wave and the one after it** pre-generated at all times. Starting a
wave promotes the pre-generated wave to active and rolls a fresh wave two ahead, so the upcoming
wave's exact composition is known in advance (the HUD reads it through the wave manager).

---

## Complete example

```json
{
    "budget":   { "type": "step_linear", "base_budget": 20, "linear_step": 10, "tier_adjustment": 30, "victory_wave": 0 },
    "boss":     { "interval": 20, "boss_enemies": ["sovereign"] },
    "upgrade_interval": 5,
    "enemy_pool": [
        { "enemy": "shade",     "cost": 3,  "min_wave": 1,   "interval": 1.0 },
        { "enemy": "flicker",   "cost": 4,  "min_wave": 2,   "interval": 0.5 },
        { "enemy": "glacius",   "cost": 5,  "min_wave": 3,   "interval": 1.2 },
        { "enemy": "cinder",    "cost": 6,  "min_wave": 4,   "interval": 1.2 },
        { "enemy": "golem",     "cost": 10, "min_wave": 4,   "interval": 1.8 },
        { "enemy": "brood",     "cost": 8,  "min_wave": 5,   "interval": 2.5 },
        { "enemy": "sentinel",  "cost": 9,  "min_wave": 5,   "interval": 1.2 },
        { "enemy": "titan",     "cost": 14, "min_wave": 6,   "interval": 1.6 },
        { "enemy": "sovereign", "cost": 40, "min_wave": 999, "interval": 1.0 }
    ]
}
```

This config runs endlessly (`victory_wave: 0`), introduces tougher enemies as `min_wave` thresholds
unlock, sends a lone `sovereign` (plus escorts) every 20th wave, and upgrades every enemy one tier
every 5 waves. The step-linear budget grows by `10` per wave but drops by `30` on each upgrade tier
(waves 5, 10, 15, …), so each tier sends fewer-but-stronger enemies; swap `type` to `"exponential"`
with a `growth_rate` for smooth compounding instead. Set `victory_wave` to a positive number to make
the run end after that wave is cleared.
