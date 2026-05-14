#include <factory/tower_factory.hpp>
#include <world/tower_modules.hpp>
#include <stdexcept>
#include <iostream>

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
    auto json = jsonio.Load("assets/data/towers");
    if (json.is_null() || !json.contains("towers")) {
        std::cerr << "TowerFactory: failed to load towers data\n";
        return;
    }

    for (auto& entry : json["towers"]) {
        TowerTemplate tmpl;
        tmpl.name          = entry["name"];
        tmpl.texture       = entry.value("texture", "");
        tmpl.fireRate        = entry.value("fireRate", 1.0f);
        tmpl.attackDuration = entry.value("attackDuration", 0.15f);
        tmpl.radius          = entry.value("radius", 64.0f);
        tmpl.targetCount   = entry.value("targetCount", 1);
        tmpl.attackType    = ParseAttackType(entry.value("attackType", "Line"));
        tmpl.targetingMode = ParseTargetingMode(entry.value("targetingMode", "First"));

        if (entry.contains("modules")) {
            for (auto& mod : entry["modules"]) {
                ModuleData m;
                m.type     = mod["type"];
                m.damage   = mod.value("damage", 0.0f);
                m.factor   = mod.value("factor", 1.0f);
                m.duration = mod.value("duration", 0.0f);
                tmpl.modules.push_back(m);
            }
        }

        std::string name = tmpl.name;
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
    tower.m_name          = tmpl.name;
    tower.m_texture       = tmpl.texture;
    tower.m_fireRate        = tmpl.fireRate;
    tower.m_attackDuration = tmpl.attackDuration;
    tower.m_radius          = tmpl.radius;
    tower.m_targetCount   = tmpl.targetCount;
    tower.m_attackType    = tmpl.attackType;
    tower.m_targetingMode = tmpl.targetingMode;

    for (auto& mod : tmpl.modules) {
        if (mod.type == "FlatDamage")
            tower.AddModule(std::make_unique<FlatDamageModule>(mod.damage));
        else if (mod.type == "Slow")
            tower.AddModule(std::make_unique<SlowModule>(mod.factor, mod.duration));
    }

    return tower;
}

bool TowerFactory::Has(const std::string& name) const {
    return m_templates.count(name) > 0;
}
