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

    const std::vector<std::string>& GetNames() const { return m_order; }
    int GetCost(const std::string& name) const;
    float GetRadius(const std::string& name) const;
    const std::string& GetTexture(const std::string& name) const;

private:
    struct ModuleData {
        std::string type;
        float damage = 0.0f;
        float factor = 1.0f;
        float duration = 0.0f;
        float pierce = 0.0f;
        float critChance = 0.0f;
        float critMultiplier = 2.0f;
    };

    struct TowerTemplate {
        std::string name;
        std::string description;
        std::string texture;
        int cost = 100;
        float fireRate = 1.0f;
        float attackDuration = 0.15f;
        float radius = 64.0f;
        int targetCount = 1;
        AttackType attackType = AttackType::Line;
        TargetingMode targetingMode = TargetingMode::First;
        std::vector<ModuleData> modules;
    };

    std::unordered_map<std::string, TowerTemplate> m_templates;
    std::vector<std::string> m_order;
};
