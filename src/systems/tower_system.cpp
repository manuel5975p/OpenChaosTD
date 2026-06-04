#include <systems/tower_system.hpp>

#include <raymath.h>
#include <algorithm>

#include <world/tower.hpp>
#include <world/enemy_modules.hpp>

// Cooldown sentinels (seconds). Cooldown counts down each frame; a shot is taken at <= 0.
constexpr float kIdleRetryCooldown = 0.05f;  // short retry while no target is in range
constexpr float kInactiveCooldown  = 999.0f; // shotsPerMinute == 0: effectively never fires
constexpr float kSecondsPerMinute  = 60.0f;  // shot interval = 60 / shotsPerMinute

void TowerSystem::Update(float dt, GameData& gameData, ParticleSystem& particles){
    for (Tower& tower : gameData.m_towers) {
        RecomputeStats(tower, dt); // runs for walls too (ticks their modules)

        AttackModule* attack = tower.GetAttack();
        if (!attack) continue; // no AttackModule -> a wall, never fires

        tower.m_cooldown -= dt;
        DecayAttackFlash(tower, dt);

        if (tower.m_cooldown > 0.0f) continue; // not ready to fire yet

        std::vector<DenseSlotMap<Enemy>::Key> targetKeys = FindTargets(tower, gameData.m_enemies, attack->m_liveTargetCount);
        if (targetKeys.empty()) {
            tower.m_cooldown = kIdleRetryCooldown;
            continue;
        }

        Fire(tower, targetKeys, gameData, particles);
    }
}

// Reset the AttackModule's live stats from base, then let modules tick and augment them.
void TowerSystem::RecomputeStats(Tower& tower, float dt) {
    AttackModule* attack = tower.GetAttack();
    if (attack) attack->ResetLive();
    for (auto& mod : tower.m_modules) {
        mod->Tick(dt);
        if (attack) mod->ContributeTower(*attack); // AttackModule's own override is a no-op
    }
}

// Decay the attack flash over the visual's m_attackDuration; guard against a zero duration.
void TowerSystem::DecayAttackFlash(Tower& tower, float dt) {
    if (tower.m_visual.m_attackDuration > 0.0f)
        tower.m_attackFlashRatio = std::max(0.0f, tower.m_attackFlashRatio - dt / tower.m_visual.m_attackDuration);
    else
        tower.m_attackFlashRatio = 0.0f;
}

// Execute one shot: reset cooldown/flash, notify modules, and enqueue the damage payload + VFX.
void TowerSystem::Fire(Tower& tower, const std::vector<DenseSlotMap<Enemy>::Key>& targetKeys, GameData& gameData, ParticleSystem& particles) {
    float shotsPerMinute     = tower.GetAttack()->m_liveShotsPerMinute;
    tower.m_cooldown         = (shotsPerMinute > 0.0f) ? kSecondsPerMinute / shotsPerMinute : kInactiveCooldown;
    tower.m_attackFlashRatio = 1.0f;
    for (auto& mod : tower.m_modules) // note: after the cooldown reset, so a new stack affects the next-but-one shot
        mod->OnFire();

    std::vector<Vector2> targetPositions;
    targetPositions.reserve(targetKeys.size());
    for (auto& key : targetKeys) {
        if (Enemy* e = gameData.m_enemies.Get(key))
            targetPositions.push_back(e->m_position);
    }

    AttackPayload payload;
    payload.m_targetKeys = targetKeys;
    BuildPayload(tower, payload);

    VfxEffect vfx = BuildVfx(tower, std::move(targetPositions));

    if (tower.m_visual.m_muzzleDesc.m_count > 0) // muzzle burst
        particles.Emit(tower.m_position, tower.m_visual.m_muzzleDesc);

    gameData.m_payloads.push_back(std::move(payload));
    gameData.m_vfx.push_back(std::move(vfx));
}

VfxEffect TowerSystem::BuildVfx(const Tower& tower, std::vector<Vector2> targetPositions) {
    VfxEffect vfx;
    vfx.m_origin = tower.m_position;
    vfx.m_targetPositions = std::move(targetPositions);
    vfx.m_duration = tower.m_visual.m_attackDuration;
    vfx.m_maxDuration = tower.m_visual.m_attackDuration;
    vfx.m_radius = tower.GetAttack()->m_liveRange;
    vfx.m_color = tower.m_visual.m_color;
    vfx.m_style = tower.m_visual.m_style;
    return vfx;
}

std::vector<DenseSlotMap<Enemy>::Key> TowerSystem::FindTargets(const Tower& tower, DenseSlotMap<Enemy>& enemies, int max_targets) {
    std::vector<Enemy*> inRange = FindEnemiesInRange(tower, enemies);

    std::sort(inRange.begin(), inRange.end(), [&](const Enemy* a, const Enemy* b) {
        return CompareTarget(*a, *b, tower.GetAttack()->m_targetingMode);
    });

    int size = static_cast<int>(inRange.size());
    int count = (max_targets == 0) ? size : std::min(size, max_targets); // 0 = target all in range

    std::vector<DenseSlotMap<Enemy>::Key> result;
    result.reserve(count);
    for (int i = 0; i < count; i++)
        result.push_back(enemies.KeyOf(inRange[i]));

    return result;
}

std::vector<Enemy*> TowerSystem::FindEnemiesInRange(const Tower& tower, DenseSlotMap<Enemy>& enemies) {
    float range = tower.GetAttack()->m_liveRange;
    std::vector<Enemy*> result;
    for (auto& enemy : enemies) {
        if (enemy.m_currentHealth <= 0.0f) continue;
        if (range >= Vector2Distance(enemy.m_position, tower.m_position))
            result.push_back(&enemy);
    }
    return result;
}

void TowerSystem::BuildPayload(const Tower& tower, AttackPayload& payload) {
    // Damage comes from the AttackModule; armor pierce and behavioural effects come from modules
    payload.m_damage = tower.GetAttack()->m_liveDamage;
    if (tower.m_visual.m_impactDesc.m_count > 0)
        payload.m_impactDescs.push_back(tower.m_visual.m_impactDesc);
    if (tower.m_visual.m_critImpactDesc.m_count > 0)
        payload.m_critImpactDescs.push_back(tower.m_visual.m_critImpactDesc);
    for (auto& mod : tower.m_modules)
        mod->Contribute(payload);
}

// Base damage for one hit: effective armor reduction plus a single crit roll.
// outCrit reports whether this hit crit (used for crit-only impact particles).
static float ResolveDamage(const AttackPayload& payload, const Enemy& enemy, bool& outCrit) {
    float armor = std::max(0.0f, enemy.m_stats.m_armor - payload.m_armorPierce);
    float dmg = payload.m_damage;
    outCrit = payload.m_critChance > 0.0f
        && GetRandomValue(0, 99) < (int)(payload.m_critChance * 100.0f);
    if (outCrit)
        dmg *= payload.m_critMultiplier;
    return std::max(0.0f, dmg - armor);
}

// A pre-existing Weakness adds flat bonus damage to this hit, then is consumed (0 if none).
static float ConsumeWeaknessBonus(Enemy& enemy) {
    if (Effect* w = enemy.FindEffect(EffectType::Weakness)) {
        float bonus = w->m_value;
        enemy.RemoveEffect(EffectType::Weakness);
        return bonus;
    }
    return 0.0f;
}

// Post-damage effect bookkeeping: any hit wakes a stunned enemy, then the payload's own effects
// are applied last so a weakness/stun tower never consumes the effect it just applied.
static void ApplyOnHitEffects(const AttackPayload& payload, Enemy& enemy) {
    enemy.RemoveEffect(EffectType::Stun);
    for (auto& effect : payload.m_effects)
        enemy.AddEffect(effect);
}

// Impact particles — all modules contribute their burst; crit adds extra descs.
static void EmitImpact(ParticleSystem& particles, const Enemy& enemy, bool crit, const AttackPayload& payload) {
    for (auto& desc : payload.m_impactDescs)
        particles.Emit(enemy.m_position, desc);
    if (crit) {
        for (auto& desc : payload.m_critImpactDescs)
            particles.Emit(enemy.m_position, desc);
    }
}

void TowerSystem::TickPayloads(GameData& gameData, ParticleSystem& particles) {
    for (auto& payload : gameData.m_payloads) {
        if (payload.m_resolved) continue;

        for (auto& key : payload.m_targetKeys) {
            Enemy* enemy = gameData.m_enemies.Get(key);
            if (!enemy) continue;

            bool crit = false;
            float net = ResolveDamage(payload, *enemy, crit);
            net += ConsumeWeaknessBonus(*enemy); // before interception/health, per effect rules
            for (auto& mod : enemy->m_modules)
                net = mod->InterceptDamage(net);
            enemy->m_currentHealth -= net;
            ApplyOnHitEffects(payload, *enemy); // stun cleared after damage; new effects applied last
            EmitImpact(particles, *enemy, crit, payload);
        }
        payload.m_resolved = true;
    }
    std::erase_if(gameData.m_payloads, [](const AttackPayload& p) { return p.m_resolved; });
}

void TowerSystem::TickVfx(float dt, GameData& gameData) {
    for (auto& vfx : gameData.m_vfx)
        vfx.m_duration -= dt;
    std::erase_if(gameData.m_vfx, [](const VfxEffect& v) { return v.m_duration <= 0.0f; });
}

static float TotalShield(const Enemy& enemy) {
    float total = 0.0f;
    for (auto& mod : enemy.m_modules)
        total += mod->GetShield();
    return total;
}

bool TowerSystem::CompareTarget(const Enemy& a, const Enemy& b, TargetingMode mode) {
    switch (mode) {
        case TargetingMode::First:          return a.m_progress < b.m_progress;
        case TargetingMode::Last:           return a.m_progress > b.m_progress;
        case TargetingMode::MostHealth:     return a.m_currentHealth > b.m_currentHealth;
        case TargetingMode::LowestHealth:   return a.m_currentHealth < b.m_currentHealth;
        case TargetingMode::Fastest:        return a.m_stats.m_speed < b.m_stats.m_speed;
        case TargetingMode::Slowest:        return a.m_stats.m_speed > b.m_stats.m_speed;
        case TargetingMode::MostArmor:      return a.m_stats.m_armor > b.m_stats.m_armor;
        case TargetingMode::MostShield:     return TotalShield(a) > TotalShield(b);
    }
    return false;
}
