#include <world/tower_modules.hpp>
#include <world/attack.hpp>
#include <world/effect.hpp>
#include <cstdio>

// --- AttackModule ---

void AttackModule::ResetLive() {
    m_liveDamage = m_damage;
    m_liveShotsPerMinute = m_shotsPerMinute;
    m_liveRange = m_range;
    m_liveTargetCount = m_targetCount;
}

void AttackModule::PatchStats(const std::string& key, float v, bool mul) {
    if      (key == "damage")         ApplyDelta(m_damage, v, mul);
    else if (key == "shotsPerMinute") ApplyDelta(m_shotsPerMinute, v, mul);
    else if (key == "range")          ApplyDelta(m_range, v, mul);
    else if (key == "targetCount") {
        float t = static_cast<float>(m_targetCount);
        ApplyDelta(t, v, mul);
        m_targetCount = static_cast<int>(t + 0.5f);
    }
}

void AttackModule::DescribeStats(std::vector<DescLine>& out) const {
    char buf[40];
    snprintf(buf, sizeof(buf), "Damage:  %g", m_liveDamage);
    out.push_back({buf, RAYWHITE});
    snprintf(buf, sizeof(buf), "Range:   %.0f", m_liveRange);
    out.push_back({buf, RAYWHITE});
    snprintf(buf, sizeof(buf), "Rate:    %d/min", static_cast<int>(m_liveShotsPerMinute + 0.5f));
    out.push_back({buf, RAYWHITE});
    snprintf(buf, sizeof(buf), "Targets: %d", m_liveTargetCount);
    out.push_back({buf, RAYWHITE});
}

// --- ArmorPierceModule ---

ArmorPierceModule::ArmorPierceModule(float amount)
    : m_amount(amount) {}

void ArmorPierceModule::Contribute(AttackPayload& attack) const {
    attack.m_armorPierce += m_amount;
}

void ArmorPierceModule::DescribeStats(std::vector<DescLine>& out) const {
    char buf[40];
    snprintf(buf, sizeof(buf), "Pierce:  %g", m_amount);
    out.push_back({buf, GOLD});
}

void ArmorPierceModule::PatchStats(const std::string& key, float v, bool mul) {
    if (key == "armorPierce") ApplyDelta(m_amount, v, mul);
}

// --- SlowModule ---

SlowModule::SlowModule(float slowPercent, float duration, EmitterDesc particleDesc)
    : m_slowPercent(slowPercent), m_duration(duration), m_particleDesc(std::move(particleDesc)) {}

void SlowModule::Contribute(AttackPayload& attack) const {
    Effect e(EffectType::Slow, m_duration, m_slowPercent);
    e.m_particleDesc = m_particleDesc;
    attack.m_effects.push_back(std::move(e));
}

void SlowModule::DescribeStats(std::vector<DescLine>& out) const {
    char buf[40];
    snprintf(buf, sizeof(buf), "Slow:    %.0f%%  %.1fs", m_slowPercent, m_duration);
    out.push_back({buf, SKYBLUE});
}

void SlowModule::PatchStats(const std::string& key, float v, bool mul) {
    if      (key == "slowPercent")  ApplyDelta(m_slowPercent, v, mul);
    else if (key == "slowDuration") ApplyDelta(m_duration, v, mul);
}

// --- BurnModule ---

BurnModule::BurnModule(float value, float duration, EmitterDesc particleDesc)
    : m_damage(value), m_duration(duration), m_particleDesc(std::move(particleDesc)) {}

void BurnModule::Contribute(AttackPayload& attack) const {
    Effect e(EffectType::Burn, m_duration, m_damage);
    e.m_particleDesc = m_particleDesc;
    attack.m_effects.push_back(std::move(e));
}

void BurnModule::DescribeStats(std::vector<DescLine>& out) const {
    char buf[40];
    snprintf(buf, sizeof(buf), "Burn:    %g/s  %.1fs", m_damage, m_duration);
    out.push_back({buf, ORANGE});
}

void BurnModule::PatchStats(const std::string& key, float v, bool mul) {
    if      (key == "burnDamage")   ApplyDelta(m_damage, v, mul);
    else if (key == "burnDuration") ApplyDelta(m_duration, v, mul);
}

// --- ArmorShredModule ---

ArmorShredModule::ArmorShredModule(float amount, float duration, EmitterDesc particleDesc)
    : m_amount(amount), m_duration(duration), m_particleDesc(std::move(particleDesc)) {}

void ArmorShredModule::Contribute(AttackPayload& attack) const {
    Effect e(EffectType::ArmorShred, m_duration, m_amount);
    e.m_particleDesc = m_particleDesc;
    attack.m_effects.push_back(std::move(e));
}

void ArmorShredModule::DescribeStats(std::vector<DescLine>& out) const {
    char buf[40];
    snprintf(buf, sizeof(buf), "Shred:   %g  %.1fs", m_amount, m_duration);
    out.push_back({buf, GRAY});
}

void ArmorShredModule::PatchStats(const std::string& key, float v, bool mul) {
    if      (key == "shredAmount")   ApplyDelta(m_amount, v, mul);
    else if (key == "shredDuration") ApplyDelta(m_duration, v, mul);
}

// --- WeaknessModule ---

WeaknessModule::WeaknessModule(float amount, float duration, EmitterDesc particleDesc)
    : m_amount(amount), m_duration(duration), m_particleDesc(std::move(particleDesc)) {}

void WeaknessModule::Contribute(AttackPayload& attack) const {
    Effect e(EffectType::Weakness, m_duration, m_amount);
    e.m_particleDesc = m_particleDesc;
    attack.m_effects.push_back(std::move(e));
}

void WeaknessModule::DescribeStats(std::vector<DescLine>& out) const {
    char buf[40];
    snprintf(buf, sizeof(buf), "Weak:    +%g  %.1fs", m_amount, m_duration);
    out.push_back({buf, PURPLE});
}

void WeaknessModule::PatchStats(const std::string& key, float v, bool mul) {
    if      (key == "weaknessAmount")   ApplyDelta(m_amount, v, mul);
    else if (key == "weaknessDuration") ApplyDelta(m_duration, v, mul);
}

// --- StunModule ---

StunModule::StunModule(float duration, EmitterDesc particleDesc)
    : m_duration(duration), m_particleDesc(std::move(particleDesc)) {}

void StunModule::Contribute(AttackPayload& attack) const {
    // m_value = duration so the "equal or stronger" rule means a longer stun wins
    Effect e(EffectType::Stun, m_duration, m_duration);
    e.m_particleDesc = m_particleDesc;
    attack.m_effects.push_back(std::move(e));
}

void StunModule::DescribeStats(std::vector<DescLine>& out) const {
    char buf[40];
    snprintf(buf, sizeof(buf), "Stun:    %.1fs", m_duration);
    out.push_back({buf, YELLOW});
}

void StunModule::PatchStats(const std::string& key, float v, bool mul) {
    if (key == "stunDuration") ApplyDelta(m_duration, v, mul);
}

// --- CritModule ---

CritModule::CritModule(float critChance, float critMultiplier)
    : m_critChance(critChance), m_critMultiplier(critMultiplier) {}

void CritModule::Contribute(AttackPayload& attack) const {
    attack.m_critChance = m_critChance;
    attack.m_critMultiplier = m_critMultiplier;
}

void CritModule::DescribeStats(std::vector<DescLine>& out) const {
    char buf[40];
    snprintf(buf, sizeof(buf), "Crit:    %.0f%%  x%.1f", m_critChance * 100.0f, m_critMultiplier);
    out.push_back({buf, YELLOW});
}

void CritModule::PatchStats(const std::string& key, float v, bool mul) {
    if      (key == "critChance")     ApplyDelta(m_critChance, v, mul);
    else if (key == "critMultiplier") ApplyDelta(m_critMultiplier, v, mul);
}

// --- RampUpModule ---

RampUpModule::RampUpModule(float bonusPerStack, int maxStacks, float idleTime)
    : m_bonusPerStack(bonusPerStack), m_idleTime(idleTime), m_maxStacks(maxStacks) {}

void RampUpModule::ContributeTower(AttackModule& attack) const {
    attack.m_liveShotsPerMinute += m_stacks * m_bonusPerStack;
}

void RampUpModule::Tick(float dt) {
    if (m_stacks <= 0) return;
    m_idleTimer += dt;
    if (m_idleTimer >= m_idleTime) { // idle too long — lose the ramp
        m_stacks = 0;
        m_idleTimer = 0.0f;
    }
}

void RampUpModule::OnFire() {
    if (m_stacks < m_maxStacks) m_stacks++;
    m_idleTimer = 0.0f;
}

void RampUpModule::DescribeStats(std::vector<DescLine>& out) const {
    char buf[40];
    snprintf(buf, sizeof(buf), "Ramp:    %d/%d", m_stacks, m_maxStacks);
    out.push_back({buf, GREEN});
}

void RampUpModule::PatchStats(const std::string& key, float v, bool mul) {
    if      (key == "bonusPerStack") ApplyDelta(m_bonusPerStack, v, mul);
    else if (key == "idleTime")      ApplyDelta(m_idleTime, v, mul);
    else if (key == "maxStacks")     m_maxStacks = mul ? static_cast<int>(m_maxStacks * v + 0.5f) : m_maxStacks + static_cast<int>(v + 0.5f);
}
