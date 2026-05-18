#include <world/enemy_modules.hpp>
#include <world/enemy.hpp>
#include <algorithm>
#include <cstdio>

// --- RegenerationModule ---

void RegenerationModule::Tick(float dt, Enemy& enemy) const {
    enemy.m_currentHealth = std::min(enemy.m_maxHealth, enemy.m_currentHealth + m_rate * dt);
}

void RegenerationModule::Describe(std::string& text, Color& color) const {
    char buf[32];
    snprintf(buf, sizeof(buf), "Regen:   %g/s", m_rate);
    text = buf;
    color = RAYWHITE;
}

// --- ArmorModule ---

void ArmorModule::ContributeStats(EnemyStats& stats) const {
    stats.armor += m_amount;
}

void ArmorModule::Describe(std::string& text, Color& color) const {
    char buf[32];
    snprintf(buf, sizeof(buf), "Armor:   %g", m_amount);
    text = buf;
    color = RAYWHITE;
}

// --- ResistanceModule ---

void ResistanceModule::ContributeStats(EnemyStats& stats) const {
    stats.resistance = std::min(1.0f, stats.resistance + m_factor);
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
        case EffectType::Slow: text = "Immune:  Slow"; color = SKYBLUE;  return;
        case EffectType::Burn: text = "Immune:  Burn"; color = ORANGE;   return;
    }
    text = "";
    color = RAYWHITE;
}

// --- ShieldModule ---

float ShieldModule::InterceptDamage(float incoming) const {
    if (m_currentShield <= 0.0f) return incoming;
    float absorbed = std::min(m_currentShield, incoming);
    m_currentShield -= absorbed;
    return incoming - absorbed;
}

void ShieldModule::Describe(std::string& text, Color& color) const {
    char buf[40];
    snprintf(buf, sizeof(buf), "Shield:  %.0f/%.0f", m_currentShield, m_maxShield);
    text = buf;
    color = {0, 220, 255, 255};
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
