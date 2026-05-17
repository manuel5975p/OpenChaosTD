#include <world/enemy_modules.hpp>
#include <world/enemy.hpp>
#include <algorithm>

void RegenerationModule::Tick(float dt, Enemy& enemy) const {
    enemy.m_currentHealth = std::min(enemy.m_health, enemy.m_currentHealth + m_rate * dt);
}

void ResistanceModule::Tick(float, Enemy& enemy) const {
    enemy.m_resistance = std::min(1.0f, enemy.m_resistance + m_factor);
}

std::optional<SpawnRequest> SplitModule::OnDeath() const {
    return SpawnRequest{m_childType, m_count};
}

bool ImmuneModule::ShouldBlock(EffectType type) const {
    return m_effect == type; 
}
