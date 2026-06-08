#include <factory/enemy_factory.hpp>
#include <world/enemy_modules.hpp>
#include <stdexcept>
#include <iostream>

static EffectType ParseEffectType(const std::string& name) {
    if (name == "Slow")       return EffectType::Slow;
    if (name == "ArmorShred") return EffectType::ArmorShred;
    if (name == "Stun")       return EffectType::Stun;
    if (name == "Weakness")   return EffectType::Weakness;
    return EffectType::Burn;
}

static EnemyPresentation ParsePresentation(const toml::table& j, const EmitterPresets& presets) {
    EnemyPresentation v;
    v.m_texture = j["texture"].value_or(std::string{});
    v.m_deathSound = j["deathSound"].value_or(std::string("enemy_death"));
    if (auto e = j["deathEmitter"].value<std::string>()) v.m_deathDescPtr = presets.GetPtr(*e);
    return v;
}

static EnemyUpgrade ParseUpgrade(const toml::table& j) {
    EnemyUpgrade up;
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

void EnemyFactory::Load(FileStore& fileStore, const EmitterPresets& presets) {
    m_builders["Regeneration"] = [](const toml::table& j){ return std::make_unique<RegenerationModule>(j["regenRate"].value_or(0.0f)); };
    m_builders["Armor"]        = [](const toml::table& j){ return std::make_unique<ArmorModule>(j["armor"].value_or(0.0f)); };
    m_builders["Immune"]       = [](const toml::table& j){ return std::make_unique<ImmuneModule>(ParseEffectType(j["effect"].value_or(std::string{}))); };
    m_builders["Shield"]       = [](const toml::table& j){ return std::make_unique<ShieldModule>(j["shield"].value_or(0.0f)); };
    m_builders["Split"]        = [](const toml::table& j){ return std::make_unique<SplitModule>(j["child"].value_or(std::string{}), j["splitCount"].value_or(0), j["spacing"].value_or(12.0f)); };

    auto data = fileStore.LoadToml("data/enemies.toml");
    auto enemies = data["enemies"].as_array();
    if (!enemies) {
        std::cerr << "EnemyFactory: failed to load enemies data\n";
        return;
    }

    for (auto&& entryNode : *enemies) {
        auto entry = entryNode.as_table();
        if (!entry) continue;

        EnemyTemplate tmpl;
        tmpl.name        = (*entry)["name"].value_or(std::string{});
        tmpl.description = (*entry)["description"].value_or(std::string{});
        tmpl.maxHealth   = (*entry)["maxHealth"].value_or(10.0f);
        tmpl.speed       = (*entry)["speed"].value_or(50.0f);
        tmpl.reward      = (*entry)["reward"].value_or(5);
        tmpl.livesOnReach = (*entry)["livesOnReach"].value_or(1);
        if (auto vis = (*entry)["presentation"].as_table()) tmpl.presentation = ParsePresentation(*vis, presets);

        if (auto mods = (*entry)["modules"].as_array())
            for (auto&& m : *mods)
                if (auto mod = m.as_table()) tmpl.modules.push_back(*mod);

        if (auto up = (*entry)["upgrade"].as_table())
            tmpl.upgrade = ParseUpgrade(*up);

        std::string name = tmpl.name;
        m_templates[name] = std::move(tmpl);
        std::cout << "EnemyFactory: loaded '" << name << "'\n";
    }
}

Enemy EnemyFactory::Create(const std::string& name) const {
    auto it = m_templates.find(name);
    if (it == m_templates.end())
        throw std::runtime_error("EnemyFactory: unknown enemy '" + name + "'");

    const EnemyTemplate& tmpl = it->second;
    Enemy enemy;
    enemy.m_name        = tmpl.name;
    enemy.m_description = tmpl.description;
    enemy.m_presentation = tmpl.presentation;
    enemy.m_upgrade     = tmpl.upgrade ? &*tmpl.upgrade : nullptr; // stable: templates are fixed after Load

    // The core stats module is always present and added first, so it is cached (GetBaseStats) and
    // contributes its base speed before any other module. The analogue of the tower's AttackModule.
    auto base = std::make_unique<BaseStatsModule>();
    base->m_maxHealth    = tmpl.maxHealth;
    base->m_speed        = tmpl.speed;
    base->m_reward       = tmpl.reward;
    base->m_livesOnReach = tmpl.livesOnReach;
    enemy.AddModule(std::move(base));
    enemy.m_currentHealth = enemy.GetBaseStats()->m_maxHealth;

    // Build every other module; AddModule caches the ShieldModule.
    for (auto& mod : tmpl.modules)
        if (auto m = BuildModule(mod))
            enemy.AddModule(std::move(m));

    // Initial live-stat pass so the HUD/targeting read valid values before the first tick
    // (mirrors TowerFactory calling AttackModule::ResetLive at build time).
    enemy.RecomputeLive();

    return enemy;
}

std::unique_ptr<EnemyModule> EnemyFactory::BuildModule(const toml::table& mod) const {
    std::string type = mod["type"].value_or(std::string{});
    auto bit = m_builders.find(type);
    if (bit != m_builders.end())
        return bit->second(mod);
    std::cerr << "EnemyFactory: unknown module type '" << type << "'\n";
    return nullptr;
}

void EnemyFactory::ApplyUpgrade(Enemy& enemy, const EnemyUpgrade& up, bool includeModules) const {
    // Each key routes through Enemy::PatchStats: base stats hit the enemy's own fields,
    // everything else is forwarded to the modules that recognize it.
    for (auto& [k, v] : up.m_adds) enemy.PatchStats(k, v, false);
    for (auto& [k, v] : up.m_muls) enemy.PatchStats(k, v, true);
    // Scalar deltas stack per tier, but added modules are appended only once across all tiers
    // (unlike tower upgrades, where each distinct level adds its modules once). The tier loop in
    // WaveManager::ApplyTierUpgrades passes includeModules=true on the first tier only.
    if (includeModules)
        for (auto& mod : up.m_addModules)
            if (auto m = BuildModule(mod)) enemy.AddModule(std::move(m));
    enemy.m_level++;
}

bool EnemyFactory::Has(const std::string& name) const {
    return m_templates.count(name) > 0;
}
