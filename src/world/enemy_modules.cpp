#include <world/enemy_modules.hpp>
#include <world/enemy.hpp>
#include <algorithm>

// --- BaseStatsModule ---

void BaseStatsModule::ResetLive() {
    m_liveSpeed = m_speed;
    m_liveArmor = 0.0f; // no innate armor; ArmorModule contributes it via ContributeStats
}

void BaseStatsModule::PatchStats(const std::string& key, float v, bool mul) {
    if      (key == "maxHealth") ApplyDelta(m_maxHealth, v, mul);
    else if (key == "speed")     ApplyDelta(m_speed, v, mul);
    else if (key == "reward")       PatchInt(m_reward, v, mul);
    else if (key == "livesOnReach") PatchInt(m_livesOnReach, v, mul);
}

void BaseStatsModule::DescribeStats(std::vector<DescLine>& out) const {
    PushStatLine(out, RAYWHITE, "Health:  %g", m_maxHealth);
    PushStatLine(out, RAYWHITE, "Speed:   %g", m_liveSpeed);
}

// --- RegenerationModule ---

void RegenerationModule::Tick(float dt, Enemy& enemy) {
    float maxHealth = enemy.GetBaseStats()->m_maxHealth;
    enemy.m_currentHealth = std::min(maxHealth, enemy.m_currentHealth + m_rate * dt);
}

void RegenerationModule::DescribeStats(std::vector<DescLine>& out) const {
    PushStatLine(out, RAYWHITE, "Regen:   %g/s", m_rate);
}

void RegenerationModule::PatchStats(const std::string& key, float v, bool mul) {
    if (key == "regenRate") ApplyDelta(m_rate, v, mul);
}

// --- ArmorModule ---

void ArmorModule::ContributeStats(BaseStatsModule& base) const {
    base.m_liveArmor += m_amount;
}

void ArmorModule::DescribeStats(std::vector<DescLine>& out) const {
    PushStatLine(out, RAYWHITE, "Armor:   %g", m_amount);
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
    PushStatLine(out, {0, 220, 255, 255}, "Shield:  %.0f/%.0f", m_currentShield, m_maxShield);
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
    return SpawnRequest{m_childType, m_count, m_spacing};
}

void SplitModule::DescribeStats(std::vector<DescLine>& out) const {
    PushStatLine(out, RAYWHITE, "Split:   %dx %s", m_count, m_childType.c_str());
}

void SplitModule::PatchStats(const std::string& key, float v, bool mul) {
    if (key == "splitCount") PatchInt(m_count, v, mul);
}
