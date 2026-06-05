#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <memory>
#include <nlohmann/json.hpp>
#include <world/enemy.hpp>
#include <engine/util/json_store.hpp>
#include <factory/emitter_presets.hpp>

class EnemyFactory {
public:
    void Load(JsonStore& jsonio, const EmitterPresets& presets);
    Enemy Create(const std::string& name) const;
    bool Has(const std::string& name) const;

    // Build a single module from its JSON description (also used to append upgrade modules).
    std::unique_ptr<EnemyModule> BuildModule(const nlohmann::json& mod) const;

    // Apply one upgrade level to an enemy: broadcast deltas via Enemy::PatchStats and append
    // any new modules. The enemy analogue of PlayingState::UpgradeSelectedTower (no gold/UI).
    void ApplyUpgrade(Enemy& enemy, const EnemyUpgrade& up) const;

private:
    using ModuleBuilder = std::function<std::unique_ptr<EnemyModule>(const nlohmann::json&)>;

    struct EnemyTemplate {
        std::string name;
        std::string description;
        std::string texture;
        float health = 10.0f;
        float speed = 50.0f;
        int reward = 5;
        int livesOnReach = 1;
        const EmitterDesc* deathDescPtr = nullptr; // stable pointer into EmitterPresets
        std::vector<nlohmann::json> modules;
        std::vector<EnemyUpgrade> upgrades;
    };

    std::unordered_map<std::string, ModuleBuilder> m_builders;
    std::unordered_map<std::string, EnemyTemplate> m_templates;
};
