#include <world/tower_modules.hpp>
#include <world/attack.hpp>
#include <world/effect.hpp>
#include <cstdio>

void FlatDamageModule::Contribute(Attack& attack) const {
    attack.m_damage += m_damage;
}

void FlatDamageModule::Describe(std::string& text, Color& color) const {
    char buf[32];
    snprintf(buf, sizeof(buf), "Damage:  %g", m_damage);
    text = buf;
    color = RAYWHITE;
}

void SlowModule::Contribute(Attack& attack) const {
    attack.m_effects.push_back(Effect(EffectType::Slow, m_duration, m_factor));
}

void SlowModule::Describe(std::string& text, Color& color) const {
    char buf[40];
    snprintf(buf, sizeof(buf), "Slow:    %.0f%%  %.1fs", (1.0f - m_factor) * 100.0f, m_duration);
    text = buf;
    color = SKYBLUE;
}

// --- BurnModule ---

void BurnModule::Contribute(Attack& attack) const {
    attack.m_effects.push_back(Effect(EffectType::Burn, m_duration, m_value));
}

void BurnModule::Describe(std::string& text, Color& color) const {
    char buf[40];
    snprintf(buf, sizeof(buf), "Burn:    %g/s  %.1fs", m_value, m_duration);
    text = buf;
    color = ORANGE;
}

// --- ArmorPiercingModule ---

void ArmorPiercingModule::Contribute(Attack& attack) const {
    attack.m_armorPierce += m_pierce;
}

void ArmorPiercingModule::Describe(std::string& text, Color& color) const {
    char buf[32];
    snprintf(buf, sizeof(buf), "Pierce:  %g", m_pierce);
    text = buf;
    color = GOLD;
}

// --- CritModule ---

void CritModule::Contribute(Attack& attack) const {
    attack.m_critChance = m_chance;
    attack.m_critMultiplier = m_multiplier;
}

void CritModule::Describe(std::string& text, Color& color) const {
    char buf[40];
    snprintf(buf, sizeof(buf), "Crit:    %.0f%%  x%.1f", m_chance * 100.0f, m_multiplier);
    text = buf;
    color = YELLOW;
}
