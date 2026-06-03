# Tower JSON Schema

All towers are defined in `data/towers.json` as an array under the `"towers"` key.

## Top-level fields

| Field         | Type   | Required | Description |
|---------------|--------|----------|-------------|
| `name`        | string | yes      | Unique identifier used in code and factory lookup |
| `description` | string | yes      | Shown in the info panel |
| `texture`     | string | yes      | Resource key for the tower sprite |
| `cost`        | int    | yes      | Gold cost to place |
| `role`        | string | no       | `"Wall"` or `"Shooter"` (default: `"Shooter"`) |

A `"Wall"` tower has no `combat`, `effects`, `visual`, or `upgrades` blocks.

---

## `combat` block

Populates `TowerStats`. All fields are optional and default to 0 / `"First"` / 1.0.

| Field           | Type   | Description |
|-----------------|--------|-------------|
| `damage`        | float  | Damage per shot |
| `shotsPerMinute`| float  | Attack cadence; cooldown = 60 / shotsPerMinute |
| `range`         | float  | Radius in world units |
| `targetCount`   | int    | Max simultaneous targets; 0 = all in range |
| `targeting`     | string | Priority rule — see targeting modes below |
| `armorPierce`   | float  | Armor value ignored before damage reduction |
| `critChance`    | float  | Probability of a critical hit (0.0–1.0) |
| `critMultiplier`| float  | Damage multiplier on crit (default 1.0) |

**Targeting modes:** `First`, `Last`, `MostHealth`, `LowestHealth`, `Fastest`, `Slowest`,
`MostArmor`, `MostShield`

---

## `effects` array

Each entry adds a behavior module to the tower (`Slow`, `Burn`, `ArmorShred`, `Weakness`,
`Stun`, or `SlowStart`). All fields inside each entry are module-specific.

**Effect rules** (apply to every status effect a module inflicts):
- Effects never stack. Reapplying refreshes the timer (and value) only when the new effect is
  **equal or stronger** (by its magnitude; for Stun the magnitude is its duration).
- Every effect expires after its `duration`.
- `ArmorShred` floors armor at 0. `Stun` and `Weakness` are also cleared the moment the enemy
  is next hit (Stun frees movement; Weakness adds its bonus damage to that hit, then is gone).

### Slow
```json
{ "type": "Slow", "slowPercent": 50, "duration": 2.0, "effect": "slow_effect" }
```
| Field      | Type   | Description |
|------------|--------|-------------|
| `slowPercent` | float | Slow strength as a percent (90 = 90% slower) |
| `duration` | float  | Seconds the slow lasts |
| `effect`   | string | Emitter preset name for the on-enemy particle effect |

### Burn
```json
{ "type": "Burn", "damage": 0.5, "duration": 8.0, "effect": "burn_effect" }
```
| Field      | Type   | Description |
|------------|--------|-------------|
| `damage`   | float  | Damage per second while burning |
| `duration` | float  | Seconds the burn lasts |
| `effect`   | string | Emitter preset name for the on-enemy particle effect |

### ArmorShred
```json
{ "type": "ArmorShred", "amount": 2, "duration": 4.0, "effect": "shred_effect" }
```
| Field      | Type   | Description |
|------------|--------|-------------|
| `amount`   | float  | Flat armor removed while active (enemy armor floored at 0) |
| `duration` | float  | Seconds the shred lasts |
| `effect`   | string | Emitter preset name for the on-enemy particle effect |

### Weakness
```json
{ "type": "Weakness", "amount": 8, "duration": 5.0, "effect": "weakness_effect" }
```
| Field      | Type   | Description |
|------------|--------|-------------|
| `amount`   | float  | Flat bonus damage the next hit deals; consumed on that hit |
| `duration` | float  | Seconds the weakness lasts if not consumed |
| `effect`   | string | Emitter preset name for the on-enemy particle effect |

### Stun
```json
{ "type": "Stun", "duration": 1.0, "effect": "stun_effect" }
```
| Field      | Type   | Description |
|------------|--------|-------------|
| `duration` | float  | Seconds the enemy can't move; also cleared the moment it's next hit |
| `effect`   | string | Emitter preset name for the on-enemy particle effect |

### SlowStart
```json
{ "type": "SlowStart", "bonusPerStack": 18, "maxStacks": 8, "idleTime": 1.2 }
```
A self-buff on the tower (no enemy effect). Each shot adds a stack up to `maxStacks`; each stack
raises `shotsPerMinute` by `bonusPerStack`. All stacks clear after `idleTime` seconds of not firing.
Pair with a `"mul": { "shotsPerMinute": 0.5 }` on the same upgrade for a slow-start, ramping feel.

| Field           | Type  | Description |
|-----------------|-------|-------------|
| `bonusPerStack` | float | Flat shotsPerMinute added per stack |
| `maxStacks`     | int   | Stack cap |
| `idleTime`      | float | Seconds without firing before all stacks are lost |

---

## `visual` block

Controls beam/ring appearance and particle bursts. Does not affect gameplay.

| Field           | Type        | Description |
|-----------------|-------------|-------------|
| `style`         | string      | `"Line"` (beam to each target) or `"Ring"` (area pulse) |
| `attackDuration`| float       | Seconds the beam/ring is visible; also controls muzzle-flash fade |
| `color`         | [R,G,B,A]   | RGBA color of the beam or ring (0–255 per channel) |
| `muzzle`        | string      | Emitter preset fired at the tower origin on attack |
| `impact`        | string      | Emitter preset fired at each target on hit |
| `critImpact`    | string      | Additional emitter preset fired on critical hits |

---

## `upgrades` array

Each entry is one purchasable upgrade level, applied in order (L1 first, then L2, etc.).

```json
{ "cost": 90, "add": { ... }, "mul": { ... }, "effects": [ ... ] }
```

| Field     | Type   | Description |
|-----------|--------|-------------|
| `cost`    | int    | Gold required to purchase |
| `add`     | object | Additive deltas — key→value pairs |
| `mul`     | object | Multiplicative factors — key→value pairs |
| `effects` | array  | New modules to append (same schema as the top-level `effects` array) |

All three optional fields are independent and can appear together in one upgrade level.

### `add` / `mul` keys

**TowerStats fields** (applied to the tower's base stats):

| Key             | Affects |
|-----------------|---------|
| `damage`        | Damage per shot |
| `shotsPerMinute`| Attack cadence |
| `range`         | Attack radius |
| `targetCount`   | Max simultaneous targets |
| `armorPierce`   | Armor penetration |
| `critChance`    | Critical hit probability |
| `critMultiplier`| Critical damage multiplier |

**Module parameters** (broadcast to all installed modules; ignored by modules that don't handle the key):

| Key                | Module           | Affects |
|--------------------|------------------|---------|
| `slowPercent`      | SlowModule       | Slow strength % (higher = stronger slow) |
| `slowDuration`     | SlowModule       | Seconds the slow lasts |
| `burnDamage`       | BurnModule       | Damage per second while burning |
| `burnDuration`     | BurnModule       | Seconds the burn lasts |
| `shredAmount`      | ArmorShredModule | Flat armor removed |
| `shredDuration`    | ArmorShredModule | Seconds the shred lasts |
| `weaknessAmount`   | WeaknessModule   | Flat bonus damage on the next hit |
| `weaknessDuration` | WeaknessModule   | Seconds the weakness lasts |
| `stunDuration`     | StunModule       | Seconds the stun lasts |
| `bonusPerStack`    | SlowStartModule  | shotsPerMinute added per stack |
| `maxStacks`        | SlowStartModule  | Stack cap |
| `idleTime`         | SlowStartModule  | Seconds idle before stacks clear |
