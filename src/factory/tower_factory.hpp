#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <memory>
#include <toml++/toml.hpp>
#include <world/tower.hpp>
#include <engine/util/file_store.hpp>
#include <factory/emitter_presets.hpp>

class TowerFactory {
public:
    void Load(FileStore& fileStore, const EmitterPresets& presets);
    Tower Create(const std::string& name) const;
    bool Has(const std::string& name) const;

    const std::vector<std::string>& GetNames() const { return m_order; }
    int GetCost(const std::string& name) const;
    float GetRange(const std::string& name) const;
    const std::string& GetTexture(const std::string& name) const;

    // Build one effect module from a TOML definition (shared by Create and upgrades).
    std::unique_ptr<TowerModule> BuildModule(const toml::table& mod) const;

private:
    using ModuleBuilder = std::function<std::unique_ptr<TowerModule>(const toml::table&)>;

    // Look up the optional "effect" emitter preset on a module definition (default if absent).
    EmitterDesc ResolveEmitter(const toml::table& j) const;

    struct TowerTemplate {
        std::string name;
        std::string description;
        int cost = 100;
        float range = 0.0f; // cached from the Attack module def for GetRange (build preview/ghost)
        TowerPresentation visual;
        std::vector<toml::table> modules; // all module defs (Attack/Passive/effects), built per Create
        std::vector<TowerUpgrade> upgrades;
    };

    const EmitterPresets* m_presets = nullptr;
    std::unordered_map<std::string, ModuleBuilder> m_builders;
    std::unordered_map<std::string, TowerTemplate> m_templates;
    std::vector<std::string> m_order;
};
