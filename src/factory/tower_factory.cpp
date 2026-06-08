#include <factory/tower_factory.hpp>
#include <world/tower_modules.hpp>
#include <stdexcept>
#include <iostream>

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

static Color ParseColor(const toml::array& a) {
    return {
        (unsigned char)a[0].value_or(0), (unsigned char)a[1].value_or(0),
        (unsigned char)a[2].value_or(0), (unsigned char)a[3].value_or(0)
    };
}

static AttackStyle ParseAttackStyle(const std::string& s) {
    if (s == "Ring") return AttackStyle::Ring;
    return AttackStyle::Line;
}

// Build the combat "Attack" module (formerly the `combat` block / TowerStats).
static std::unique_ptr<TowerModule> BuildAttackModule(const toml::table& j) {
    auto a = std::make_unique<AttackModule>();
    a->m_damage         = j["damage"].value_or(0.0f);
    a->m_shotsPerMinute = j["shotsPerMinute"].value_or(0.0f);
    a->m_range          = j["range"].value_or(0.0f);
    a->m_targetCount    = j["targetCount"].value_or(0);
    a->m_targetingMode  = ParseTargetingMode(j["targetingMode"].value_or(std::string("First")));
    a->ResetLive(); // live==base until the first RecomputeStats tick (HUD may read it sooner)
    return a;
}

static TowerPresentation ParsePresentation(const toml::table& j, const EmitterPresets& presets) {
    TowerPresentation v;
    v.m_texture = j["texture"].value_or(std::string{});
    v.m_attackSound = j["attackSound"].value_or(std::string{});
    v.m_style = ParseAttackStyle(j["style"].value_or(std::string("Line")));
    v.m_attackDuration = j["attackDuration"].value_or(0.0f);
    if (auto c = j["color"].as_array())            v.m_color          = ParseColor(*c);
    if (auto m = j["muzzle"].value<std::string>()) v.m_muzzleDesc     = presets.Get(*m);
    if (auto m = j["impact"].value<std::string>()) v.m_impactDesc     = presets.Get(*m);
    if (auto m = j["critImpact"].value<std::string>()) v.m_critImpactDesc = presets.Get(*m);
    return v;
}

static TowerUpgrade ParseUpgrade(const toml::table& j) {
    TowerUpgrade up;
    up.m_cost = j["cost"].value_or(0);
    if (auto add = j["add"].as_table())
        for (auto&& [k, v] : *add) up.m_adds.push_back({std::string(k.str()), v.value_or(0.0f)});
    if (auto mul = j["mul"].as_table())
        for (auto&& [k, v] : *mul) up.m_muls.push_back({std::string(k.str()), v.value_or(0.0f)});
    // Added modules live under "modules" (unified upgrade schema across towers and enemies).
    if (auto mods = j["modules"].as_array())
        for (auto&& m : *mods)
            if (auto mt = m.as_table()) up.m_addModules.push_back(*mt);
    return up;
}

void TowerFactory::Load(FileStore& fileStore, const EmitterPresets& presets) {
    m_presets = &presets;

    m_builders["Attack"] = [](const toml::table& j){
        return BuildAttackModule(j);
    };
    m_builders["Passive"] = [](const toml::table&){
        return std::make_unique<PassiveModule>();
    };
    m_builders["ArmorPierce"] = [](const toml::table& j){
        return std::make_unique<ArmorPierceModule>(j["armorPierce"].value_or(0.0f));
    };
    m_builders["Slow"] = [this](const toml::table& j){
        return std::make_unique<SlowModule>(j["slowPercent"].value_or(0.0f), j["slowDuration"].value_or(0.0f), ResolveEmitter(j));
    };
    m_builders["Burn"] = [this](const toml::table& j){
        return std::make_unique<BurnModule>(j["burnDamage"].value_or(0.0f), j["burnDuration"].value_or(0.0f), ResolveEmitter(j));
    };
    m_builders["ArmorShred"] = [this](const toml::table& j){
        return std::make_unique<ArmorShredModule>(j["shredAmount"].value_or(0.0f), j["shredDuration"].value_or(0.0f), ResolveEmitter(j));
    };
    m_builders["Weakness"] = [this](const toml::table& j){
        return std::make_unique<WeaknessModule>(j["weaknessAmount"].value_or(0.0f), j["weaknessDuration"].value_or(0.0f), ResolveEmitter(j));
    };
    m_builders["Stun"] = [this](const toml::table& j){
        return std::make_unique<StunModule>(j["stunDuration"].value_or(0.0f), ResolveEmitter(j));
    };
    m_builders["RampUp"] = [](const toml::table& j){
        return std::make_unique<RampUpModule>(j["bonusPerStack"].value_or(0.0f), j["maxStacks"].value_or(0), j["idleTime"].value_or(1.0f));
    };
    m_builders["Crit"] = [](const toml::table& j){
        return std::make_unique<CritModule>(j["critChance"].value_or(0.0f), j["critMultiplier"].value_or(1.0f));
    };

    auto data = fileStore.LoadToml("data/towers.toml");
    auto towers = data["towers"].as_array();
    if (!towers) {
        std::cerr << "TowerFactory: failed to load towers data\n";
        return;
    }

    for (auto&& entryNode : *towers) {
        auto entry = entryNode.as_table();
        if (!entry) continue;

        TowerTemplate tmpl;
        tmpl.name        = (*entry)["name"].value_or(std::string{});
        tmpl.description = (*entry)["description"].value_or(std::string{});
        tmpl.cost        = (*entry)["cost"].value_or(100);

        // Texture now lives inside the presentation block (mirroring EnemyPresentation); ParsePresentation reads it.
        if (auto vis = (*entry)["presentation"].as_table()) tmpl.presentation = ParsePresentation(*vis, presets);

        // Every behaviour is a module now: Attack (combat), Passive (wall marker), or an effect.
        if (auto mods = (*entry)["modules"].as_array())
            for (auto&& m : *mods)
                if (auto mod = m.as_table()) {
                    if ((*mod)["type"].value_or(std::string{}) == "Attack")
                        tmpl.range = (*mod)["range"].value_or(0.0f); // cached for GetRange
                    tmpl.modules.push_back(*mod);
                }

        if (auto ups = (*entry)["upgrades"].as_array())
            for (auto&& u : *ups)
                if (auto up = u.as_table())
                    tmpl.upgrades.push_back(ParseUpgrade(*up));

        std::string name = tmpl.name;
        m_order.push_back(name);
        m_templates[name] = std::move(tmpl);
        std::cout << "TowerFactory: loaded '" << name << "'\n";
    }
}

EmitterDesc TowerFactory::ResolveEmitter(const toml::table& j) const {
    EmitterDesc effect;
    if (auto e = j["effect"].value<std::string>()) effect = m_presets->Get(*e);
    return effect;
}

std::unique_ptr<TowerModule> TowerFactory::BuildModule(const toml::table& mod) const {
    std::string type = mod["type"].value_or(std::string{});
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
    tower.m_presentation = tmpl.presentation; // includes the texture key
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
    return (it != m_templates.end()) ? it->second.presentation.m_texture : empty;
}
