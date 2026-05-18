#include <factory/tower_factory.hpp>
#include <world/tower_modules.hpp>
#include <stdexcept>
#include <iostream>

using json = nlohmann::json;

static AttackType ParseAttackType(const std::string& s) {
    if (s == "Area") return AttackType::Area;
    return AttackType::Line;
}

static TargetingMode ParseTargetingMode(const std::string& s) {
    if (s == "Last")         return TargetingMode::Last;
    if (s == "MostHealth")   return TargetingMode::MostHealth;
    if (s == "LowestHealth") return TargetingMode::LowestHealth;
    if (s == "Fastest")      return TargetingMode::Fastest;
    if (s == "Slowest")      return TargetingMode::Slowest;
    return TargetingMode::First;
}

void TowerFactory::Load(JsonIO& jsonio) {
    m_builders["FlatDamage"]    = [](const json& j){ return std::make_unique<FlatDamageModule>(j.value("damage", 0.0f)); };
    m_builders["Slow"]          = [](const json& j){ return std::make_unique<SlowModule>(j.value("factor", 1.0f), j.value("duration", 0.0f)); };
    m_builders["Burn"]          = [](const json& j){ return std::make_unique<BurnModule>(j.value("damage", 0.0f), j.value("duration", 0.0f)); };
    m_builders["ArmorPiercing"] = [](const json& j){ return std::make_unique<ArmorPiercingModule>(j.value("pierce", 0.0f)); };
    m_builders["Crit"]          = [](const json& j){ return std::make_unique<CritModule>(j.value("critChance", 0.0f), j.value("critMultiplier", 2.0f)); };

    auto data = jsonio.Load("data/towers.json");
    if (data.is_null() || !data.contains("towers")) {
        std::cerr << "TowerFactory: failed to load towers data\n";
        return;
    }

    for (auto& entry : data["towers"]) {
        TowerTemplate tmpl;
        tmpl.name            = entry["name"];
        tmpl.description     = entry.value("description", "");
        tmpl.texture         = entry.value("texture", "");
        tmpl.cost            = entry.value("cost", 100);
        tmpl.fireRate        = entry.value("fireRate", 1.0f);
        tmpl.attackDuration  = entry.value("attackDuration", 0.15f);
        tmpl.radius          = entry.value("radius", 64.0f);
        tmpl.targetCount     = entry.value("targetCount", 1);
        tmpl.attackType      = ParseAttackType(entry.value("attackType", "Line"));
        tmpl.targetingMode   = ParseTargetingMode(entry.value("targetingMode", "First"));

        if (entry.contains("modules")) {
            for (auto& mod : entry["modules"])
                tmpl.modules.push_back(mod);
        }

        std::string name = tmpl.name;
        m_order.push_back(name);
        m_templates[name] = std::move(tmpl);
        std::cout << "TowerFactory: loaded '" << name << "'\n";
    }
}

Tower TowerFactory::Create(const std::string& name) const {
    auto it = m_templates.find(name);
    if (it == m_templates.end())
        throw std::runtime_error("TowerFactory: unknown tower '" + name + "'");

    const TowerTemplate& tmpl = it->second;
    Tower tower;
    tower.m_name             = tmpl.name;
    tower.m_description      = tmpl.description;
    tower.m_texture          = tmpl.texture;
    tower.m_cost             = tmpl.cost;
    tower.m_base.radius         = tmpl.radius;
    tower.m_base.fireRate       = tmpl.fireRate;
    tower.m_base.attackDuration = tmpl.attackDuration;
    tower.m_base.targetCount    = tmpl.targetCount;
    tower.m_base.attackType     = tmpl.attackType;
    tower.m_base.targetingMode  = tmpl.targetingMode;
    tower.m_stats = tower.m_base;

    for (auto& mod : tmpl.modules) {
        std::string type = mod.value("type", "");
        auto bit = m_builders.find(type);
        if (bit != m_builders.end())
            tower.AddModule(bit->second(mod));
        else
            std::cerr << "TowerFactory: unknown module type '" << type << "'\n";
    }

    return tower;
}

bool TowerFactory::Has(const std::string& name) const {
    return m_templates.count(name) > 0;
}

int TowerFactory::GetCost(const std::string& name) const {
    auto it = m_templates.find(name);
    return (it != m_templates.end()) ? it->second.cost : 0;
}

float TowerFactory::GetRadius(const std::string& name) const {
    auto it = m_templates.find(name);
    return (it != m_templates.end()) ? it->second.radius : 0.0f;
}

const std::string& TowerFactory::GetTexture(const std::string& name) const {
    static const std::string empty;
    auto it = m_templates.find(name);
    return (it != m_templates.end()) ? it->second.texture : empty;
}
