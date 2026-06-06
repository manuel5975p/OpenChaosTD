# Enemy JSON Schema

All enemies are defined in `data/enemies.json` as an array under the `"enemies"` key.

## Top-level fields

| Field          | Type   | Required | Description |
|----------------|--------|----------|-------------|
| `name`         | string | yes      | Unique identifier used in code and factory lookup |
| `description`  | string | yes      | Shown in the info panel |
| `maxHealth`    | float  | yes      | Starting and maximum hit points |
| `speed`        | float  | yes      | Movement speed in world units per second |
| `reward`       | int    | yes      | Gold granted to the player when the enemy is killed |
| `livesOnReach` | int    | no       | Lives lost if the enemy reaches the core (default 1) |
| `visual`       | object | yes      | Presentation-only data (sprite, death effect) — see below |
| `modules`      | array  | no       | The enemy's defensive and mechanical traits — see below |
| `upgrade`      | object | no       | A single stat-scaling step re-applied once per upgrade tier — see below |

The core stats above (`maxHealth`, `speed`, `reward`, `livesOnReach`) stay top-level in JSON but are
parsed into an internal core stats module at build time — the runtime analogue of the tower's
`Attack` module — so their names match the `upgrade` keys exactly.

An enemy with no `modules` is a plain target defined entirely by its core stats (`shade` and
`flicker` are examples). All special behavior — armor, regeneration, shields, splitting, and
status immunities — comes from the `modules` array.

---

## `visual` object

All presentation-only data lives in a nested `"visual"` object, separate from the gameplay
stats. This mirrors the `"visual"` block used by towers (`data/towers.md`).

```json
"visual": {
    "texture": "enemy_shade",
    "deathEmitter": "death_small"
}
```

| Field          | Type   | Required | Description |
|----------------|--------|----------|-------------|
| `texture`      | string | yes      | Resource key for the enemy sprite |
| `deathEmitter` | string | no       | Emitter preset spawned at the enemy's position on death |

---

## `modules` array

Each entry is one module, identified by its `"type"`. All fields inside an entry are
module-specific. The available module types are `Armor`, `Regeneration`, `Shield`, `Split`,
and `Immune`. An enemy may combine any number of them (the `sovereign` stacks four).

### Armor

```json
{ "type": "Armor", "armor": 3.0 }
```

Reduces every incoming hit by a flat amount. The reduction is applied after any tower armor
pierce, and the resulting damage is floored at 0 — so an enemy can never be healed by an
attack that is fully absorbed by armor.

| Field   | Type  | Description |
|---------|-------|-------------|
| `armor` | float | Flat damage subtracted from each incoming hit |

### Regeneration

```json
{ "type": "Regeneration", "regenRate": 2.0 }
```

Restores health every frame while the enemy is alive. Healing is capped at the enemy's
maximum health.

| Field       | Type  | Description |
|-------------|-------|-------------|
| `regenRate` | float | Health restored per second |

### Shield

```json
{ "type": "Shield", "shield": 15.0 }
```

A depletable damage pool that absorbs incoming damage before any is dealt to health. Each hit
drains the shield first; once it reaches 0 the remaining damage passes through to health. The
shield does not recharge on its own.

| Field    | Type  | Description |
|----------|-------|-------------|
| `shield` | float | Starting and maximum shield pool |

### Split

```json
{ "type": "Split", "child": "golem", "splitCount": 2 }
```

On death, spawns `splitCount` child enemies of type `child` at the dying enemy's position. The
`child` value must be the `name` of another enemy defined in `enemies.json`.

| Field        | Type   | Description |
|--------------|--------|-------------|
| `child`      | string | `name` of the enemy type to spawn on death |
| `splitCount` | int    | Number of children to spawn |

### Immune

```json
{ "type": "Immune", "effect": "Stun" }
```

Blocks a single status-effect type from ever being applied to the enemy. Towers can still
hit and damage the enemy normally; only the named effect is ignored. Add one module per
effect to grant multiple immunities.

| Field    | Type   | Description |
|----------|--------|-------------|
| `effect` | string | Status effect to ignore — see effect types below |

**Effect types:** `Slow`, `Burn`, `ArmorShred`, `Stun`, `Weakness`

---

## `upgrade` object

An optional single upgrade step that scales an enemy beyond its base definition (e.g. for
elite or late-wave variants). Applying it broadcasts its deltas through the enemy's
stat-patching pipeline and appends any new modules.

```json
"upgrade": { "cost": 1, "add": { "armor": 2, "regenRate": 2 }, "mul": { "maxHealth": 1.5 } }
```

| Field     | Type   | Description |
|-----------|--------|-------------|
| `cost`    | int    | Reserved for a future "threat budget"; currently unused by the runtime |
| `add`     | object | Map of stat key → **flat** delta added to the current value |
| `mul`     | object | Map of stat key → **multiplier** applied to the current value |
| `modules` | array  | Additional `modules` entries appended to the enemy each time the upgrade is applied |

### How the upgrade scales over tiers

Waves carry an **upgrade tier** (configured in `data/waves.json` via `upgrade_interval` — the tier
increments every Nth wave). The wave manager applies this one upgrade definition **once per tier**:
tier 1 applies it once, tier 2 applies it twice, and so on, with no upper bound (endless mode keeps
stacking). Because the deltas are re-applied on top of the already-upgraded stats, the effect
compounds — `add` accumulates linearly while `mul` compounds exponentially. For example, the
`add: { "armor": 2 }`, `mul: { "maxHealth": 1.5 }` step above yields at tier *n*: `+2n` armor and
`maxHealth × 1.5ⁿ`. An enemy with no `upgrade` key never scales and ignores the wave tier entirely.

`add` and `mul` accept the same keys, routed to either the enemy's base stats or the matching
module:

| Key            | Target | Notes |
|----------------|--------|-------|
| `maxHealth`    | core   | Also refills current health, so a scaled enemy spawns at full HP |
| `speed`        | core   | Recomputed into the live stat on the next tick |
| `reward`       | core   | Rounded to the nearest integer |
| `livesOnReach` | core   | Rounded to the nearest integer |
| `armor`        | module | Requires an `Armor` module |
| `regenRate`    | module | Requires a `Regeneration` module |
| `shield`       | module | Requires a `Shield` module; also tops up the live shield pool |
| `splitCount`   | module | Requires a `Split` module; rounded to the nearest integer |

The core stats (`maxHealth`/`speed`/`reward`/`livesOnReach`) route to the enemy's core stats module;
the rest route to the matching trait module. A key with no matching field or module is silently ignored.

---

## Damage resolution order

When a tower hits an enemy, damage is resolved in this order:

1. **Armor** subtracts its `armor` value (after the attacker's armor pierce); the result is floored at 0.
2. **Shield** absorbs what remains, draining its pool before any damage reaches health.
3. Whatever is left is deducted from the enemy's health.

`Regeneration` runs independently each frame, and `Immune` simply prevents matching status
effects from being applied at all.
