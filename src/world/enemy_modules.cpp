#include <world/enemy_modules.hpp>
#include <world/enemy.hpp>
#include <algorithm>
#include <cstdio>

// --- RegenerationModule ---

void RegenerationModule::Tick(float dt, Enemy& enemy) const {
    enemy.m_currentHealth = std::min(enemy.m_health, enemy.m_currentHealth + m_rate * dt);
}

std::string RegenerationModule::Describe() const {
    char buf[32];
    snprintf(buf, sizeof(buf), "Regen:   %g/s", m_rate);
    return buf;
}

// --- ArmorModule ---

float ArmorModule::GetArmor() const {
    return m_amount;
}

std::string ArmorModule::Describe() const {
    char buf[32];
    snprintf(buf, sizeof(buf), "Armor:   %g", m_amount);
    return buf;
}

// --- ResistanceModule ---

void ResistanceModule::Tick(float, Enemy& enemy) const {
    enemy.m_resistance = std::min(1.0f, enemy.m_resistance + m_factor);
}

std::string ResistanceModule::Describe() const {
    char buf[32];
    snprintf(buf, sizeof(buf), "Resist:  %.0f%%", m_factor * 100.0f);
    return buf;
}

// --- ImmuneModule ---

bool ImmuneModule::ShouldBlock(EffectType type) const {
    return m_effect == type;
}

std::string ImmuneModule::Describe() const {
    switch (m_effect) {
        case EffectType::Slow: return "Immune:  Slow";
        case EffectType::Burn: return "Immune:  Burn";
    }
    return "";
}

// --- SplitModule ---

std::optional<SpawnRequest> SplitModule::OnDeath() const {
    return SpawnRequest{m_childType, m_count};
}

std::string SplitModule::Describe() const {
    char buf[48];
    snprintf(buf, sizeof(buf), "Split:   %dx %s", m_count, m_childType.c_str());
    return buf;
}
