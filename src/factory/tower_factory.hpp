#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <memory>
#include <nlohmann/json.hpp>
#include <world/tower.hpp>
#include <core/jsonio.hpp>
#include <factory/emitter_presets.hpp>

class TowerFactory {
public:
    void Load(JsonIO& jsonio, const EmitterPresets& presets);
    Tower Create(const std::string& name) const;
    bool Has(const std::string& name) const;

    const std::vector<std::string>& GetNames() const { return m_order; }
    int GetCost(const std::string& name) const;
    float GetRadius(const std::string& name) const;
    const std::string& GetTexture(const std::string& name) const;

private:
    using ModuleBuilder = std::function<std::unique_ptr<TowerModule>(const nlohmann::json&)>;

    struct TowerTemplate {
        std::string name;
        std::string description;
        std::string texture;
        int cost = 100;
        float fireRate = 1.0f;
        float attackDuration = 0.15f;
        float radius = 64.0f;
        int targetCount = 1;
        TargetingMode targetingMode = TargetingMode::First;
        std::vector<nlohmann::json> modules;
        TowerVfxDesc vfx;
    };

    std::unordered_map<std::string, ModuleBuilder> m_builders;
    std::unordered_map<std::string, TowerTemplate> m_templates;
    std::vector<std::string> m_order;
};
