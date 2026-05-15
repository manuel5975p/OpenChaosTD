#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <world/enemy.hpp>
#include <core/jsonio.hpp>

class EnemyFactory {
public:
    void Load(JsonIO& jsonio);
    Enemy Create(const std::string& name) const;
    bool Has(const std::string& name) const;

private:
    struct ModuleData {
        std::string type;
        float rate   = 0.0f;
        float amount = 0.0f;
        float factor = 0.0f;
    };

    struct EnemyTemplate {
        std::string name;
        std::string description;
        std::string texture;
        float health = 10.0f;
        float speed = 50.0f;
        int reward = 5;
        int livesOnReach = 1;
        std::vector<ModuleData> modules;
    };

    std::unordered_map<std::string, EnemyTemplate> m_templates;
};
