#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <world/tower.hpp>
#include <core/jsonio.hpp>

class TowerFactory {
public:
    void Load(JsonIO& jsonio);
    Tower Create(const std::string& name) const;
    bool Has(const std::string& name) const;

private:
    struct ModuleData {
        std::string type;
        float damage   = 0.0f;
        float factor   = 1.0f;
        float duration = 0.0f;
    };

    struct TowerTemplate {
        std::string   name;
        std::string   texture;
        float         fireRate      = 1.0f;
        float         radius        = 64.0f;
        int           targetCount   = 1;
        AttackType    attackType    = AttackType::Line;
        TargetingMode targetingMode = TargetingMode::First;
        std::vector<ModuleData> modules;
    };

    std::unordered_map<std::string, TowerTemplate> m_templates;
};
