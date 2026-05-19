#include <world/tower_modules.hpp>
#include <world/attack.hpp>
#include <world/effect.hpp>
#include <cstdio>

void FlatDamageModule::Contribute(AttackPayload& attack) const {
    attack.m_damage += m_damage;
}

void FlatDamageModule::Describe(std::string& text, Color& color) const {
    char buf[32];
    snprintf(buf, sizeof(buf), "Damage:  %g", m_damage);
    text = buf;
    color = RAYWHITE;
}

// --- SlowModule ---

SlowModule::SlowModule(float factor, float duration) : m_factor(factor), m_duration(duration) {
    m_emitRate = 8.0f;
    m_particleDesc.color = {160, 220, 255, 180};
    m_particleDesc.endColor = {80, 160, 220, 0};
    m_particleDesc.count = 1;
    m_particleDesc.spread = 3.14159f;
    m_particleDesc.speed = 10.0f;
    m_particleDesc.speedVariance = 4.0f;
    m_particleDesc.lifetime = 0.38f;
    m_particleDesc.size = 2.0f;
    m_particleDesc.endSize = 0.0f;
}

void SlowModule::Contribute(AttackPayload& attack) const {
    Effect e(EffectType::Slow, m_duration, m_factor);
    e.m_particleDesc = m_particleDesc;
    e.m_emitRate = m_emitRate;
    attack.m_effects.push_back(std::move(e));
}

void SlowModule::Describe(std::string& text, Color& color) const {
    char buf[40];
    snprintf(buf, sizeof(buf), "Slow:    %.0f%%  %.1fs", (1.0f - m_factor) * 100.0f, m_duration);
    text = buf;
    color = SKYBLUE;
}

// --- BurnModule ---

BurnModule::BurnModule(float value, float duration) : m_value(value), m_duration(duration) {
    m_emitRate = 18.0f;
    m_particleDesc.color = {255, 110, 30, 255};
    m_particleDesc.endColor = {180, 40, 0, 0};
    m_particleDesc.count = 1;
    m_particleDesc.angle = 3.14159f * 1.5f;    // upward
    m_particleDesc.spread = 0.5f;
    m_particleDesc.speed = 22.0f;
    m_particleDesc.speedVariance = 8.0f;
    m_particleDesc.lifetime = 0.28f;
    m_particleDesc.size = 2.5f;
    m_particleDesc.endSize = 0.0f;
}

void BurnModule::Contribute(AttackPayload& attack) const {
    Effect e(EffectType::Burn, m_duration, m_value);
    e.m_particleDesc = m_particleDesc;
    e.m_emitRate = m_emitRate;
    attack.m_effects.push_back(std::move(e));
}

void BurnModule::Describe(std::string& text, Color& color) const {
    char buf[40];
    snprintf(buf, sizeof(buf), "Burn:    %g/s  %.1fs", m_value, m_duration);
    text = buf;
    color = ORANGE;
}

// --- ArmorPiercingModule ---

void ArmorPiercingModule::Contribute(AttackPayload& attack) const {
    attack.m_armorPierce += m_pierce;
}

void ArmorPiercingModule::Describe(std::string& text, Color& color) const {
    char buf[32];
    snprintf(buf, sizeof(buf), "Pierce:  %g", m_pierce);
    text = buf;
    color = GOLD;
}

// --- CritModule ---

void CritModule::Contribute(AttackPayload& attack) const {
    attack.m_critChance = m_chance;
    attack.m_critMultiplier = m_multiplier;
}

void CritModule::Describe(std::string& text, Color& color) const {
    char buf[40];
    snprintf(buf, sizeof(buf), "Crit:    %.0f%%  x%.1f", m_chance * 100.0f, m_multiplier);
    text = buf;
    color = YELLOW;
}
