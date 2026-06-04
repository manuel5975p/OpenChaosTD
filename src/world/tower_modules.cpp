#include <world/tower_modules.hpp>
#include <world/attack.hpp>
#include <world/effect.hpp>
#include <cstdio>

// --- SlowModule ---

SlowModule::SlowModule(float slowPercent, float duration, EmitterDesc particleDesc)
    : m_slowPercent(slowPercent), m_duration(duration), m_particleDesc(std::move(particleDesc)) {}

void SlowModule::Contribute(AttackPayload& attack) const {
    Effect e(EffectType::Slow, m_duration, m_slowPercent);
    e.m_particleDesc = m_particleDesc;
    e.m_emitRate = m_emitRate;
    attack.m_effects.push_back(std::move(e));
}

void SlowModule::Describe(std::string& text, Color& color) const {
    char buf[40];
    snprintf(buf, sizeof(buf), "Slow:    %.0f%%  %.1fs", m_slowPercent, m_duration);
    text = buf;
    color = SKYBLUE;
}

void SlowModule::PatchStat(const std::string& key, float v, bool mul) {
    if      (key == "slowPercent")  ApplyDelta(m_slowPercent, v, mul);
    else if (key == "slowDuration") ApplyDelta(m_duration, v, mul);
}

// --- BurnModule ---

BurnModule::BurnModule(float value, float duration, EmitterDesc particleDesc)
    : m_damage(value), m_duration(duration), m_particleDesc(std::move(particleDesc)) {}

void BurnModule::Contribute(AttackPayload& attack) const {
    Effect e(EffectType::Burn, m_duration, m_damage);
    e.m_particleDesc = m_particleDesc;
    e.m_emitRate = m_emitRate;
    attack.m_effects.push_back(std::move(e));
}

void BurnModule::Describe(std::string& text, Color& color) const {
    char buf[40];
    snprintf(buf, sizeof(buf), "Burn:    %g/s  %.1fs", m_damage, m_duration);
    text = buf;
    color = ORANGE;
}

void BurnModule::PatchStat(const std::string& key, float v, bool mul) {
    if      (key == "burnDamage")   ApplyDelta(m_damage, v, mul);
    else if (key == "burnDuration") ApplyDelta(m_duration, v, mul);
}

// --- ArmorShredModule ---

ArmorShredModule::ArmorShredModule(float amount, float duration, EmitterDesc particleDesc)
    : m_amount(amount), m_duration(duration), m_particleDesc(std::move(particleDesc)) {}

void ArmorShredModule::Contribute(AttackPayload& attack) const {
    Effect e(EffectType::ArmorShred, m_duration, m_amount);
    e.m_particleDesc = m_particleDesc;
    e.m_emitRate = m_emitRate;
    attack.m_effects.push_back(std::move(e));
}

void ArmorShredModule::Describe(std::string& text, Color& color) const {
    char buf[40];
    snprintf(buf, sizeof(buf), "Shred:   %g  %.1fs", m_amount, m_duration);
    text = buf;
    color = GRAY;
}

void ArmorShredModule::PatchStat(const std::string& key, float v, bool mul) {
    if      (key == "shredAmount")   ApplyDelta(m_amount, v, mul);
    else if (key == "shredDuration") ApplyDelta(m_duration, v, mul);
}

// --- WeaknessModule ---

WeaknessModule::WeaknessModule(float amount, float duration, EmitterDesc particleDesc)
    : m_amount(amount), m_duration(duration), m_particleDesc(std::move(particleDesc)) {}

void WeaknessModule::Contribute(AttackPayload& attack) const {
    Effect e(EffectType::Weakness, m_duration, m_amount);
    e.m_particleDesc = m_particleDesc;
    e.m_emitRate = m_emitRate;
    attack.m_effects.push_back(std::move(e));
}

void WeaknessModule::Describe(std::string& text, Color& color) const {
    char buf[40];
    snprintf(buf, sizeof(buf), "Weak:    +%g  %.1fs", m_amount, m_duration);
    text = buf;
    color = PURPLE;
}

void WeaknessModule::PatchStat(const std::string& key, float v, bool mul) {
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
    e.m_emitRate = m_emitRate;
    attack.m_effects.push_back(std::move(e));
}

void StunModule::Describe(std::string& text, Color& color) const {
    char buf[40];
    snprintf(buf, sizeof(buf), "Stun:    %.1fs", m_duration);
    text = buf;
    color = YELLOW;
}

void StunModule::PatchStat(const std::string& key, float v, bool mul) {
    if (key == "stunDuration") ApplyDelta(m_duration, v, mul);
}

// --- CritModule ---

CritModule::CritModule(float critChance, float critMultiplier)
    : m_critChance(critChance), m_critMultiplier(critMultiplier) {}

void CritModule::Contribute(AttackPayload& attack) const {
    attack.m_critChance = m_critChance;
    attack.m_critMultiplier = m_critMultiplier;
}

void CritModule::Describe(std::string& text, Color& color) const {
    char buf[40];
    snprintf(buf, sizeof(buf), "Crit:    %.0f%%  x%.1f", m_critChance * 100.0f, m_critMultiplier);
    text = buf;
    color = YELLOW;
}

void CritModule::PatchStat(const std::string& key, float v, bool mul) {
    if      (key == "critChance")     ApplyDelta(m_critChance, v, mul);
    else if (key == "critMultiplier") ApplyDelta(m_critMultiplier, v, mul);
}

// --- SlowStartModule ---

SlowStartModule::SlowStartModule(float bonusPerStack, int maxStacks, float idleTime)
    : m_bonusPerStack(bonusPerStack), m_idleTime(idleTime), m_maxStacks(maxStacks) {}

void SlowStartModule::ContributeTower(TowerStats& stats) const {
    stats.m_shotsPerMinute += m_stacks * m_bonusPerStack;
}

void SlowStartModule::Tick(float dt) {
    if (m_stacks <= 0) return;
    m_idleTimer += dt;
    if (m_idleTimer >= m_idleTime) { // idle too long — lose the ramp
        m_stacks = 0;
        m_idleTimer = 0.0f;
    }
}

void SlowStartModule::OnFire() {
    if (m_stacks < m_maxStacks) m_stacks++;
    m_idleTimer = 0.0f;
}

void SlowStartModule::Describe(std::string& text, Color& color) const {
    char buf[40];
    snprintf(buf, sizeof(buf), "Ramp:    %d/%d", m_stacks, m_maxStacks);
    text = buf;
    color = GREEN;
}

void SlowStartModule::PatchStat(const std::string& key, float v, bool mul) {
    if      (key == "bonusPerStack") ApplyDelta(m_bonusPerStack, v, mul);
    else if (key == "idleTime")      ApplyDelta(m_idleTime, v, mul);
    else if (key == "maxStacks")     m_maxStacks = mul ? static_cast<int>(m_maxStacks * v + 0.5f) : m_maxStacks + static_cast<int>(v + 0.5f);
}
