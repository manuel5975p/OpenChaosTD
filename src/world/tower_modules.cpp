#include <world/tower_modules.hpp>
#include <world/attack.hpp>
#include <world/effect.hpp>
#include <cstdio>

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

void SlowModule::PatchStat(const std::string& key, float v, bool mul) {
    auto apply = [&](float& f) { f = mul ? f * v : f + v; };
    if      (key == "slowFactor")   apply(m_factor);
    else if (key == "slowDuration") apply(m_duration);
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

void BurnModule::PatchStat(const std::string& key, float v, bool mul) {
    auto apply = [&](float& f) { f = mul ? f * v : f + v; };
    if      (key == "burnDamage")   apply(m_value);
    else if (key == "burnDuration") apply(m_duration);
}
