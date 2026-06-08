#include <factory/enemy_factory.hpp>
#include <world/enemy_modules.hpp>
#include <stdexcept>
#include <iostream>

using json = nlohmann::json;

static EffectType ParseEffectType(const std::string& name) {
    if (name == "Slow")       return EffectType::Slow;
    if (name == "ArmorShred") return EffectType::ArmorShred;
    if (name == "Stun")       return EffectType::Stun;
    if (name == "Weakness")   return EffectType::Weakness;
    return EffectType::Burn;
}

static EnemyPresentation ParsePresentation(const json& j, const EmitterPresets& presets) {
    EnemyPresentation v;
    v.m_texture = j.value("texture", "");
    v.m_deathSound = j.value("deathSound", "enemy_death");
    if (j.contains("deathEmitter")) v.m_deathDescPtr = presets.GetPtr(j["deathEmitter"]);
    return v;
}

static EnemyUpgrade ParseUpgrade(const json& j) {
    EnemyUpgrade up;
    if (j.contains("add"))
        for (auto& [k, v] : j["add"].items()) up.m_adds.push_back({k, v.get<float>()});
    if (j.contains("mul"))
        for (auto& [k, v] : j["mul"].items()) up.m_muls.push_back({k, v.get<float>()});
    // Added modules live under "modules" (unified upgrade schema across towers and enemies).
    if (j.contains("modules"))
        for (auto& m : j["modules"]) up.m_addModules.push_back(m);
    return up;
}

void EnemyFactory::Load(JsonStore& jsonio, const EmitterPresets& presets) {
    m_builders["Regeneration"] = [](const json& j){ return std::make_unique<RegenerationModule>(j.value("regenRate", 0.0f)); };
    m_builders["Armor"]        = [](const json& j){ return std::make_unique<ArmorModule>(j.value("armor", 0.0f)); };
    m_builders["Immune"]       = [](const json& j){ return std::make_unique<ImmuneModule>(ParseEffectType(j.value("effect", ""))); };
    m_builders["Shield"]       = [](const json& j){ return std::make_unique<ShieldModule>(j.value("shield", 0.0f)); };
    m_builders["Split"]        = [](const json& j){ return std::make_unique<SplitModule>(j.value("child", ""), j.value("splitCount", 0), j.value("spacing", 12.0f)); };

    auto data = jsonio.Load("data/enemies.json");
    if (data.is_null() || !data.contains("enemies")) {
        std::cerr << "EnemyFactory: failed to load enemies data\n";
        return;
    }

    for (auto& entry : data["enemies"]) {
        EnemyTemplate tmpl;
        tmpl.name        = entry["name"];
        tmpl.description = entry.value("description", "");
        tmpl.maxHealth   = entry.value("maxHealth", 10.0f);
        tmpl.speed       = entry.value("speed", 50.0f);
        tmpl.reward      = entry.value("reward", 5);
        tmpl.livesOnReach = entry.value("livesOnReach", 1);
        if (entry.contains("visual")) tmpl.visual = ParsePresentation(entry["visual"], presets);

        if (entry.contains("modules")) {
            for (auto& mod : entry["modules"])
                tmpl.modules.push_back(mod);
        }

        if (entry.contains("upgrade"))
            tmpl.upgrade = ParseUpgrade(entry["upgrade"]);

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
    enemy.m_presentation = tmpl.visual;
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

std::unique_ptr<EnemyModule> EnemyFactory::BuildModule(const json& mod) const {
    std::string type = mod.value("type", "");
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
