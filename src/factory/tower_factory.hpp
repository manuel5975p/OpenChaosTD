#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <memory>
#include <nlohmann/json.hpp>
#include <world/tower.hpp>
#include <engine/util/json_store.hpp>
#include <factory/emitter_presets.hpp>

class TowerFactory {
public:
    void Load(JsonStore& jsonio, const EmitterPresets& presets);
    Tower Create(const std::string& name) const;
    bool Has(const std::string& name) const;

    const std::vector<std::string>& GetNames() const { return m_order; }
    int GetCost(const std::string& name) const;
    float GetRange(const std::string& name) const;
    const std::string& GetTexture(const std::string& name) const;

    // Build one effect module from a JSON definition (shared by Create and upgrades).
    std::unique_ptr<TowerModule> BuildModule(const nlohmann::json& mod) const;

private:
    using ModuleBuilder = std::function<std::unique_ptr<TowerModule>(const nlohmann::json&)>;

    struct TowerTemplate {
        std::string name;
        std::string description;
        std::string texture;
        int cost = 100;
        TowerRole role = TowerRole::Shooter;
        TowerStats stats;
        TowerVisual visual;
        std::vector<nlohmann::json> modules;
        std::vector<TowerUpgrade> upgrades;
    };

    const EmitterPresets* m_presets = nullptr;
    std::unordered_map<std::string, ModuleBuilder> m_builders;
    std::unordered_map<std::string, TowerTemplate> m_templates;
    std::vector<std::string> m_order;
};
