#include <world/tower_modules.hpp>
#include <world/attack.hpp>
#include <world/effect.hpp>
#include <cstdio>

void FlatDamageModule::Contribute(Attack& attack) const {
    attack.m_damage += m_damage;
}

std::string FlatDamageModule::Describe() const {
    char buf[32];
    snprintf(buf, sizeof(buf), "Damage:  %g", m_damage);
    return buf;
}

void SlowModule::Contribute(Attack& attack) const {
    attack.m_effects.push_back(Effect(EffectType::Slow, m_duration, m_factor));
}

std::string SlowModule::Describe() const {
    char buf[40];
    snprintf(buf, sizeof(buf), "Slow:    %.0f%%  %.1fs", (1.0f - m_factor) * 100.0f, m_duration);
    return buf;
}

Color SlowModule::DescribeColor() const {
    return SKYBLUE;
}
