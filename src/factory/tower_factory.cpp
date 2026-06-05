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
    if (s == "MostShield") return TargetingMode::MostShield;
    return TargetingMode::First;
}

static Color ParseColor(const json& j) {
    return {
        (unsigned char)j[0].get<int>(), (unsigned char)j[1].get<int>(),
        (unsigned char)j[2].get<int>(), (unsigned char)j[3].get<int>()
    };
}

static AttackStyle ParseAttackStyle(const std::string& s) {
    if (s == "Ring") return AttackStyle::Ring;
    return AttackStyle::Line;
}

// Build the combat "Attack" module (formerly the `combat` block / TowerStats).
static std::unique_ptr<TowerModule> BuildAttackModule(const json& j) {
    auto a = std::make_unique<AttackModule>();
    a->m_damage         = j.value("damage", 0.0f);
    a->m_shotsPerMinute = j.value("shotsPerMinute", 0.0f);
    a->m_range          = j.value("range", 0.0f);
    a->m_targetCount    = j.value("targetCount", 0);
    a->m_targetingMode  = ParseTargetingMode(j.value("targetingMode", "First"));
    a->ResetLive(); // live==base until the first RecomputeStats tick (HUD may read it sooner)
    return a;
}

static TowerVisual ParseVisual(const json& j, const EmitterPresets& presets) {
    TowerVisual v;
    v.m_texture = j.value("texture", "");
    v.m_style = ParseAttackStyle(j.value("style", "Line"));
    v.m_attackDuration = j.value("attackDuration", 0.0f);
    if (j.contains("color"))      v.m_color          = ParseColor(j["color"]);
    if (j.contains("muzzle"))     v.m_muzzleDesc     = presets.Get(j["muzzle"].get<std::string>());
    if (j.contains("impact"))     v.m_impactDesc     = presets.Get(j["impact"].get<std::string>());
    if (j.contains("critImpact")) v.m_critImpactDesc = presets.Get(j["critImpact"].get<std::string>());
    return v;
}

static TowerUpgrade ParseUpgrade(const json& j) {
    TowerUpgrade up;
    up.m_cost = j.value("cost", 0);
    if (j.contains("add"))
        for (auto& [k, v] : j["add"].items()) up.m_adds.push_back({k, v.get<float>()});
    if (j.contains("mul"))
        for (auto& [k, v] : j["mul"].items()) up.m_muls.push_back({k, v.get<float>()});
    if (j.contains("effects"))
        for (auto& m : j["effects"]) up.m_addModules.push_back(m);
    return up;
}

void TowerFactory::Load(JsonStore& jsonio, const EmitterPresets& presets) {
    m_presets = &presets;

    m_builders["Attack"] = [](const json& j){
        return BuildAttackModule(j);
    };
    m_builders["Passive"] = [](const json&){
        return std::make_unique<PassiveModule>();
    };
    m_builders["ArmorPierce"] = [](const json& j){
        return std::make_unique<ArmorPierceModule>(j.value("armorPierce", 0.0f));
    };
    m_builders["Slow"] = [this](const json& j){
        return std::make_unique<SlowModule>(j.value("slowPercent", 0.0f), j.value("slowDuration", 0.0f), ResolveEmitter(j));
    };
    m_builders["Burn"] = [this](const json& j){
        return std::make_unique<BurnModule>(j.value("burnDamage", 0.0f), j.value("burnDuration", 0.0f), ResolveEmitter(j));
    };
    m_builders["ArmorShred"] = [this](const json& j){
        return std::make_unique<ArmorShredModule>(j.value("shredAmount", 0.0f), j.value("shredDuration", 0.0f), ResolveEmitter(j));
    };
    m_builders["Weakness"] = [this](const json& j){
        return std::make_unique<WeaknessModule>(j.value("weaknessAmount", 0.0f), j.value("weaknessDuration", 0.0f), ResolveEmitter(j));
    };
    m_builders["Stun"] = [this](const json& j){
        return std::make_unique<StunModule>(j.value("stunDuration", 0.0f), ResolveEmitter(j));
    };
    m_builders["RampUp"] = [](const json& j){
        return std::make_unique<RampUpModule>(j.value("bonusPerStack", 0.0f), j.value("maxStacks", 0), j.value("idleTime", 1.0f));
    };
    m_builders["Crit"] = [](const json& j){
        return std::make_unique<CritModule>(j.value("critChance", 0.0f), j.value("critMultiplier", 1.0f));
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
        tmpl.cost        = entry.value("cost", 100);

        // Texture now lives inside the visual block (mirroring EnemyVisual); ParseVisual reads it.
        if (entry.contains("visual")) tmpl.visual = ParseVisual(entry["visual"], presets);

        // Every behaviour is a module now: Attack (combat), Passive (wall marker), or an effect.
        if (entry.contains("modules"))
            for (auto& mod : entry["modules"]) {
                if (mod.value("type", "") == "Attack")
                    tmpl.range = mod.value("range", 0.0f); // cached for GetRange
                tmpl.modules.push_back(mod);
            }

        if (entry.contains("upgrades"))
            for (auto& up : entry["upgrades"])
                tmpl.upgrades.push_back(ParseUpgrade(up));

        std::string name = tmpl.name;
        m_order.push_back(name);
        m_templates[name] = std::move(tmpl);
        std::cout << "TowerFactory: loaded '" << name << "'\n";
    }
}

EmitterDesc TowerFactory::ResolveEmitter(const json& j) const {
    EmitterDesc effect;
    if (j.contains("effect")) effect = m_presets->Get(j["effect"].get<std::string>());
    return effect;
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
    tower.m_cost        = tmpl.cost;
    tower.m_visual      = tmpl.visual; // includes the texture key
    tower.m_upgrades = &tmpl.upgrades; // stable: templates are fixed after Load

    // Build every module (incl. the Attack/Passive marker); AddModule caches the AttackModule.
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
    return (it != m_templates.end()) ? it->second.range : 0.0f;
}

const std::string& TowerFactory::GetTexture(const std::string& name) const {
    static const std::string empty;
    auto it = m_templates.find(name);
    return (it != m_templates.end()) ? it->second.visual.m_texture : empty;
}
