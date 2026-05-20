#include <factory/tower_factory.hpp>
#include <world/tower_modules.hpp>
#include <stdexcept>
#include <iostream>

using json = nlohmann::json;

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
    if (s == "Burst") return VfxStyle::Burst;
    if (s == "Ring")  return VfxStyle::Ring;
    if (s == "Zap")   return VfxStyle::Zap;
    return VfxStyle::Beam;
}

void TowerFactory::Load(JsonIO& jsonio, const EmitterPresets& presets) {
    // Resolve fixed effect visuals once; capture by value into the builder lambdas
    EmitterDesc slowDesc = presets.Get("slow_effect");
    EmitterDesc burnDesc = presets.Get("burn_effect");

    m_builders["FlatDamage"]    = [](const json& j){ return std::make_unique<FlatDamageModule>(j.value("damage", 0.0f)); };
    m_builders["Slow"]          = [slowDesc](const json& j){ return std::make_unique<SlowModule>(j.value("factor", 1.0f), j.value("duration", 0.0f), slowDesc); };
    m_builders["Burn"]          = [burnDesc](const json& j){ return std::make_unique<BurnModule>(j.value("damage", 0.0f), j.value("duration", 0.0f), burnDesc); };
    m_builders["ArmorPiercing"] = [](const json& j){ return std::make_unique<ArmorPiercingModule>(j.value("pierce", 0.0f)); };
    m_builders["Crit"]          = [](const json& j){ return std::make_unique<CritModule>(j.value("critChance", 0.0f), j.value("critMultiplier", 2.0f)); };

    auto data = jsonio.Load("data/towers.json");
    if (data.is_null() || !data.contains("towers")) {
        std::cerr << "TowerFactory: failed to load towers data\n";
        return;
    }

    for (auto& entry : data["towers"]) {
        TowerTemplate tmpl;
        tmpl.name           = entry["name"];
        tmpl.description    = entry.value("description", "");
        tmpl.texture        = entry.value("texture", "");
        tmpl.cost           = entry.value("cost", 100);
        tmpl.fireRate       = entry.value("fireRate", 1.0f);
        tmpl.attackDuration = entry.value("attackDuration", 0.15f);
        tmpl.radius         = entry.value("radius", 64.0f);
        tmpl.targetCount    = entry.value("targetCount", 1);
        tmpl.targetingMode  = ParseTargetingMode(entry.value("targetingMode", "First"));

        if (entry.contains("modules")) {
            for (auto& mod : entry["modules"])
                tmpl.modules.push_back(mod);
        }

        if (entry.contains("vfx")) {
            const auto& vj = entry["vfx"];
            tmpl.vfx.style = ParseVfxStyle(vj.value("style", "Beam"));
            if (vj.contains("color"))      tmpl.vfx.color = ParseColor(vj["color"]);
            if (vj.contains("muzzle"))     tmpl.vfx.muzzleDesc = presets.Get(vj["muzzle"]);
            if (vj.contains("impact"))     tmpl.vfx.impactDesc = presets.Get(vj["impact"]);
            if (vj.contains("critImpact")) tmpl.vfx.critImpactDesc = presets.Get(vj["critImpact"]);
            if (vj.contains("trail")) {
                const auto& tj = vj["trail"];
                tmpl.vfx.trailRate = tj.value("rate", 0.0f);
                if (tj.contains("preset"))
                    tmpl.vfx.trailDesc = presets.Get(tj["preset"]);
            }
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
    tower.m_name        = tmpl.name;
    tower.m_description = tmpl.description;
    tower.m_texture     = tmpl.texture;
    tower.m_cost        = tmpl.cost;
    tower.m_base.radius         = tmpl.radius;
    tower.m_base.fireRate       = tmpl.fireRate;
    tower.m_base.attackDuration = tmpl.attackDuration;
    tower.m_base.targetCount   = tmpl.targetCount;
    tower.m_base.targetingMode = tmpl.targetingMode;
    tower.m_stats = tower.m_base;
    tower.m_vfx   = tmpl.vfx;

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
