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
`MostArmor`, `MostResistance`, `MostShield`

---

## `effects` array

Each entry adds a behavior module (`SlowModule` or `BurnModule`) to the tower.
All fields inside each entry are module-specific.

### Slow
```json
{ "type": "Slow", "factor": 0.5, "duration": 2.0, "effect": "slow_effect" }
```
| Field      | Type   | Description |
|------------|--------|-------------|
| `factor`   | float  | Speed multiplier applied to the enemy (0.5 = half speed) |
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

| Key            | Module     | Affects |
|----------------|------------|---------|
| `slowFactor`   | SlowModule | Speed multiplier (lower = stronger slow) |
| `slowDuration` | SlowModule | Seconds the slow lasts |
| `burnDamage`   | BurnModule | Damage per second while burning |
| `burnDuration` | BurnModule | Seconds the burn lasts |
