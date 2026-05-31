#include <factory/enemy_factory.hpp>
#include <world/enemy_modules.hpp>
#include <stdexcept>
#include <iostream>

using json = nlohmann::json;

static EffectType ParseEffectType(const std::string& name) {
    if (name == "Slow") return EffectType::Slow;
    return EffectType::Burn;
}

void EnemyFactory::Load(JsonStore& jsonio, const EmitterPresets& presets) {
    m_builders["Regeneration"] = [](const json& j){ return std::make_unique<RegenerationModule>(j.value("rate", 0.0f)); };
    m_builders["Armor"]        = [](const json& j){ return std::make_unique<ArmorModule>(j.value("amount", 0.0f)); };
    m_builders["Resistance"]   = [](const json& j){ return std::make_unique<ResistanceModule>(j.value("factor", 0.0f)); };
    m_builders["Immune"]       = [](const json& j){ return std::make_unique<ImmuneModule>(ParseEffectType(j.value("effect", ""))); };
    m_builders["Shield"]       = [](const json& j){ return std::make_unique<ShieldModule>(j.value("amount", 0.0f)); };
    m_builders["Split"]        = [](const json& j){ return std::make_unique<SplitModule>(j.value("child", ""), j.value("count", 0)); };

    auto data = jsonio.Load("data/enemies.json");
    if (data.is_null() || !data.contains("enemies")) {
        std::cerr << "EnemyFactory: failed to load enemies data\n";
        return;
    }

    for (auto& entry : data["enemies"]) {
        EnemyTemplate tmpl;
        tmpl.name        = entry["name"];
        tmpl.description = entry.value("description", "");
        tmpl.texture     = entry.value("texture", "");
        tmpl.health      = entry.value("health", 10.0f);
        tmpl.speed       = entry.value("speed", 50.0f);
        tmpl.reward      = entry.value("reward", 5);
        tmpl.livesOnReach = entry.value("livesOnReach", 1);
        if (entry.contains("deathEmitter"))
            tmpl.deathDescPtr = presets.GetPtr(entry["deathEmitter"]);

        if (entry.contains("modules")) {
            for (auto& mod : entry["modules"])
                tmpl.modules.push_back(mod);
        }

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
    enemy.m_name         = tmpl.name;
    enemy.m_description  = tmpl.description;
    enemy.m_texture      = tmpl.texture;
    enemy.m_maxHealth     = tmpl.health;
    enemy.m_currentHealth = tmpl.health;
    enemy.m_base.speed    = tmpl.speed;
    enemy.m_stats         = enemy.m_base;
    enemy.m_reward       = tmpl.reward;
    enemy.m_livesOnReach = tmpl.livesOnReach;
    enemy.m_deathDescPtr = tmpl.deathDescPtr;

    for (auto& mod : tmpl.modules) {
        std::string type = mod.value("type", "");
        auto bit = m_builders.find(type);
        if (bit != m_builders.end())
            enemy.AddModule(bit->second(mod));
        else
            std::cerr << "EnemyFactory: unknown module type '" << type << "'\n";
    }

    return enemy;
}

bool EnemyFactory::Has(const std::string& name) const {
    return m_templates.count(name) > 0;
}
