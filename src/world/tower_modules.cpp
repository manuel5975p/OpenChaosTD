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

SlowModule::SlowModule(float factor, float duration, EmitterDesc particleDesc)
    : m_factor(factor), m_duration(duration), m_particleDesc(std::move(particleDesc)) {}

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

BurnModule::BurnModule(float value, float duration, EmitterDesc particleDesc)
    : m_value(value), m_duration(duration), m_particleDesc(std::move(particleDesc)) {}

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
