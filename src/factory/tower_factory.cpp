#include <factory/tower_factory.hpp>
#include <world/tower_modules.hpp>
#include <stdexcept>
#include <iostream>

using json = nlohmann::json;

static TowerRole ParseTowerRole(const std::string& s) {
    if (s == "Wall") return TowerRole::Wall;
    return TowerRole::Shooter;
}

static TargetingMode ParseTargetingMode(const std::string& s) {
    if (s == "Last") return TargetingMode::Last;
    if (s == "MostHealth") return TargetingMode::MostHealth;
    if (s == "LowestHealth") return TargetingMode::LowestHealth;
    if (s == "Fastest") return TargetingMode::Fastest;
    if (s == "Slowest") return TargetingMode::Slowest;
    if (s == "MostArmor") return TargetingMode::MostArmor;
    if (s == "MostResistance") return TargetingMode::MostResistance;
    if (s == "MostShield") return TargetingMode::MostShield;
    return TargetingMode::First;
}

static Color ParseColor(const json& j) {
    return {
        (unsigned char)j[0].get<int>(), (unsigned char)j[1].get<int>(),
        (unsigned char)j[2].get<int>(), (unsigned char)j[3].get<int>()
    };
}

static VfxStyle ParseVfxStyle(const std::string& s) {
    if (s == "Ring") return VfxStyle::Ring;
    return VfxStyle::Line;
}

static TowerStats ParseCombat(const json& j) {
    TowerStats s;
    s.damage         = j.value("damage", 0.0f);
    s.shotsPerMinute = j.value("shotsPerMinute", 0.0f);
    s.range          = j.value("range", 0.0f);
    s.targetCount    = j.value("targetCount", 0);
    s.targetingMode  = ParseTargetingMode(j.value("targeting", "First"));
    s.armorPierce    = j.value("armorPierce", 0.0f);
    s.critChance     = j.value("critChance", 0.0f);
    s.critMultiplier = j.value("critMultiplier", 1.0f);
    return s;
}

static TowerVisual ParseVisual(const json& j, const EmitterPresets& presets) {
    TowerVisual v;
    v.style = ParseVfxStyle(j.value("style", "Line"));
    v.attackDuration = j.value("attackDuration", 0.0f);
    if (j.contains("color"))      v.color          = ParseColor(j["color"]);
    if (j.contains("muzzle"))     v.muzzleDesc     = presets.Get(j["muzzle"].get<std::string>());
    if (j.contains("impact"))     v.impactDesc     = presets.Get(j["impact"].get<std::string>());
    if (j.contains("critImpact")) v.critImpactDesc = presets.Get(j["critImpact"].get<std::string>());
    return v;
}

static TowerUpgrade ParseUpgrade(const json& j) {
    TowerUpgrade up;
    up.cost = j.value("cost", 0);
    if (j.contains("add"))
        for (auto& [k, v] : j["add"].items()) up.adds.push_back({k, v.get<float>()});
    if (j.contains("mul"))
        for (auto& [k, v] : j["mul"].items()) up.muls.push_back({k, v.get<float>()});
    if (j.contains("effects"))
        for (auto& m : j["effects"]) up.addModules.push_back(m);
    return up;
}

void TowerFactory::Load(JsonStore& jsonio, const EmitterPresets& presets) {
    m_presets = &presets;

    m_builders["Slow"] = [this](const json& j){
        EmitterDesc effect;
        if (j.contains("effect")) effect = m_presets->Get(j["effect"].get<std::string>());
        return std::make_unique<SlowModule>(j.value("factor", 1.0f), j.value("duration", 0.0f), std::move(effect));
    };
    m_builders["Burn"] = [this](const json& j){
        EmitterDesc effect;
        if (j.contains("effect")) effect = m_presets->Get(j["effect"].get<std::string>());
        return std::make_unique<BurnModule>(j.value("damage", 0.0f), j.value("duration", 0.0f), std::move(effect));
    };

    auto data = jsonio.Load("data/towers.json");
    if (data.is_null() || !data.contains("towers")) {
        std::cerr << "TowerFactory: failed to load towers data\n";
        return;
    }

    for (auto& entry : data["towers"]) {
        TowerTemplate tmpl;
        tmpl.name        = entry["name"];
        tmpl.description = entry.value("description", "");
        tmpl.texture     = entry.value("texture", "");
        tmpl.cost        = entry.value("cost", 100);
        tmpl.role        = ParseTowerRole(entry.value("role", "Shooter"));

        if (entry.contains("combat")) tmpl.stats  = ParseCombat(entry["combat"]);
        if (entry.contains("visual")) tmpl.visual = ParseVisual(entry["visual"], presets);

        if (entry.contains("effects"))
            for (auto& mod : entry["effects"])
                tmpl.modules.push_back(mod);

        if (entry.contains("upgrades"))
            for (auto& up : entry["upgrades"])
                tmpl.upgrades.push_back(ParseUpgrade(up));

        std::string name = tmpl.name;
        m_order.push_back(name);
        m_templates[name] = std::move(tmpl);
        std::cout << "TowerFactory: loaded '" << name << "'\n";
    }
}

std::unique_ptr<TowerModule> TowerFactory::BuildModule(const json& mod) const {
    std::string type = mod.value("type", "");
    auto bit = m_builders.find(type);
    if (bit != m_builders.end())
        return bit->second(mod);
    std::cerr << "TowerFactory: unknown module type '" << type << "'\n";
    return nullptr;
}

Tower TowerFactory::Create(const std::string& name) const {
    auto it = m_templates.find(name);
    if (it == m_templates.end())
        throw std::runtime_error("TowerFactory: unknown tower '" + name + "'");

    const TowerTemplate& tmpl = it->second;
    Tower tower;
    tower.m_name        = tmpl.name;
    tower.m_description = tmpl.description;
    tower.m_texture     = tmpl.texture;
    tower.m_cost        = tmpl.cost;
    tower.m_role        = tmpl.role;
    tower.m_base   = tmpl.stats;
    tower.m_stats  = tower.m_base;
    tower.m_visual = tmpl.visual;
    tower.m_upgrades = &tmpl.upgrades; // stable: templates are fixed after Load

    for (auto& mod : tmpl.modules)
        if (auto m = BuildModule(mod))
            tower.AddModule(std::move(m));

    return tower;
}

bool TowerFactory::Has(const std::string& name) const {
    return m_templates.count(name) > 0;
}

int TowerFactory::GetCost(const std::string& name) const {
    auto it = m_templates.find(name);
    return (it != m_templates.end()) ? it->second.cost : 0;
}

float TowerFactory::GetRange(const std::string& name) const {
    auto it = m_templates.find(name);
    return (it != m_templates.end()) ? it->second.stats.range : 0.0f;
}

const std::string& TowerFactory::GetTexture(const std::string& name) const {
    static const std::string empty;
    auto it = m_templates.find(name);
    return (it != m_templates.end()) ? it->second.texture : empty;
}
