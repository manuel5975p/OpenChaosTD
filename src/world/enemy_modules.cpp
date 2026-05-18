#include <world/enemy_modules.hpp>
#include <world/enemy.hpp>
#include <algorithm>
#include <cstdio>

// --- RegenerationModule ---

void RegenerationModule::Tick(float dt, Enemy& enemy) const {
    enemy.m_currentHealth = std::min(enemy.m_health, enemy.m_currentHealth + m_rate * dt);
}

void RegenerationModule::Describe(std::string& text, Color& color) const {
    char buf[32];
    snprintf(buf, sizeof(buf), "Regen:   %g/s", m_rate);
    text = buf;
    color = RAYWHITE;
}

// --- ArmorModule ---

float ArmorModule::GetArmor() const {
    return m_amount;
}

void ArmorModule::Describe(std::string& text, Color& color) const {
    char buf[32];
    snprintf(buf, sizeof(buf), "Armor:   %g", m_amount);
    text = buf;
    color = RAYWHITE;
}

// --- ResistanceModule ---

void ResistanceModule::Tick(float, Enemy& enemy) const {
    enemy.m_resistance = std::min(1.0f, enemy.m_resistance + m_factor);
}

void ResistanceModule::Describe(std::string& text, Color& color) const {
    char buf[32];
    snprintf(buf, sizeof(buf), "Resist:  %.0f%%", m_factor * 100.0f);
    text = buf;
    color = RAYWHITE;
}

// --- ImmuneModule ---

bool ImmuneModule::ShouldBlock(EffectType type) const {
    return m_effect == type;
}

void ImmuneModule::Describe(std::string& text, Color& color) const {
    switch (m_effect) {
        case EffectType::Slow: text = "Immune:  Slow"; color = SKYBLUE; return;
        case EffectType::Burn: text = "Immune:  Burn"; color = ORANGE;  return;
    }
    text = "";
    color = RAYWHITE;
}

// --- SplitModule ---

std::optional<SpawnRequest> SplitModule::OnDeath() const {
    return SpawnRequest{m_childType, m_count};
}

void SplitModule::Describe(std::string& text, Color& color) const {
    char buf[48];
    snprintf(buf, sizeof(buf), "Split:   %dx %s", m_count, m_childType.c_str());
    text = buf;
    color = RAYWHITE;
}
