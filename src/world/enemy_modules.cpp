#include <world/enemy_modules.hpp>
#include <world/enemy.hpp>
#include <algorithm>
#include <cstdio>

// --- RegenerationModule ---

void RegenerationModule::Tick(float dt, Enemy& enemy) {
    enemy.m_currentHealth = std::min(enemy.m_maxHealth, enemy.m_currentHealth + m_rate * dt);
}

void RegenerationModule::DescribeStats(std::vector<DescLine>& out) const {
    char buf[32];
    snprintf(buf, sizeof(buf), "Regen:   %g/s", m_rate);
    out.push_back({buf, RAYWHITE});
}

void RegenerationModule::PatchStats(const std::string& key, float v, bool mul) {
    if (key == "regenRate") ApplyDelta(m_rate, v, mul);
}

// --- ArmorModule ---

void ArmorModule::ContributeStats(EnemyStats& stats) const {
    stats.m_armor += m_amount;
}

void ArmorModule::DescribeStats(std::vector<DescLine>& out) const {
    char buf[32];
    snprintf(buf, sizeof(buf), "Armor:   %g", m_amount);
    out.push_back({buf, RAYWHITE});
}

void ArmorModule::PatchStats(const std::string& key, float v, bool mul) {
    if (key == "armor") ApplyDelta(m_amount, v, mul);
}

// --- ImmuneModule ---

bool ImmuneModule::ShouldBlock(EffectType type) const {
    return m_effect == type;
}

void ImmuneModule::DescribeStats(std::vector<DescLine>& out) const {
    switch (m_effect) {
        case EffectType::Slow:       out.push_back({"Immune:  Slow",  SKYBLUE}); return;
        case EffectType::Burn:       out.push_back({"Immune:  Burn",  ORANGE});  return;
        case EffectType::ArmorShred: out.push_back({"Immune:  Shred", GRAY});    return;
        case EffectType::Stun:       out.push_back({"Immune:  Stun",  YELLOW});  return;
        case EffectType::Weakness:   out.push_back({"Immune:  Weak",  PURPLE});  return;
    }
}

// --- ShieldModule ---

float ShieldModule::InterceptDamage(float incoming) {
    if (m_currentShield <= 0.0f) return incoming;
    float absorbed = std::min(m_currentShield, incoming);
    m_currentShield -= absorbed;
    return incoming - absorbed;
}

void ShieldModule::DescribeStats(std::vector<DescLine>& out) const {
    char buf[40];
    snprintf(buf, sizeof(buf), "Shield:  %.0f/%.0f", m_currentShield, m_maxShield);
    out.push_back({buf, {0, 220, 255, 255}});
}

void ShieldModule::PatchStats(const std::string& key, float v, bool mul) {
    // Scale the pool and refill the live shield so a buffed shield is also topped up.
    if (key == "shield") {
        ApplyDelta(m_maxShield, v, mul);
        m_currentShield = m_maxShield;
    }
}

// --- SplitModule ---

std::optional<SpawnRequest> SplitModule::OnDeath() const {
    return SpawnRequest{m_childType, m_count};
}

void SplitModule::DescribeStats(std::vector<DescLine>& out) const {
    char buf[48];
    snprintf(buf, sizeof(buf), "Split:   %dx %s", m_count, m_childType.c_str());
    out.push_back({buf, RAYWHITE});
}

void SplitModule::PatchStats(const std::string& key, float v, bool mul) {
    if (key == "splitCount") {
        float c = static_cast<float>(m_count);
        ApplyDelta(c, v, mul);
        m_count = static_cast<int>(c + 0.5f);
    }
}
