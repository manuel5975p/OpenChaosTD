#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <memory>
#include <nlohmann/json.hpp>
#include <world/enemy.hpp>
#include <core/jsonio.hpp>
#include <factory/emitter_presets.hpp>

class EnemyFactory {
public:
    void Load(JsonIO& jsonio, const EmitterPresets& presets);
    Enemy Create(const std::string& name) const;
    bool Has(const std::string& name) const;

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
        EmitterDesc deathDesc;
        std::vector<nlohmann::json> modules;
    };

    std::unordered_map<std::string, ModuleBuilder> m_builders;
    std::unordered_map<std::string, EnemyTemplate> m_templates;
};
