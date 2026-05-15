#include <factory/enemy_factory.hpp>
#include <world/enemy_module.hpp>
#include <stdexcept>
#include <iostream>

void EnemyFactory::Load(JsonIO& jsonio) {
    auto json = jsonio.Load("data/enemies.json");
    if (json.is_null() || !json.contains("enemies")) {
        std::cerr << "EnemyFactory: failed to load enemies data\n";
        return;
    }

    for (auto& entry : json["enemies"]) {
        EnemyTemplate tmpl;
        tmpl.name = entry["name"];
        tmpl.description = entry.value("description", "");
        tmpl.texture = entry.value("texture", "");
        tmpl.health = entry.value("health", 10.0f);
        tmpl.speed = entry.value("speed", 50.0f);
        tmpl.reward = entry.value("reward", 5);
        tmpl.livesOnReach = entry.value("livesOnReach", 1);

        if (entry.contains("modules")) {
            for (auto& mod : entry["modules"]) {
                ModuleData m;
                m.type = mod["type"];
                m.rate = mod.value("rate", 0.0f);
                m.amount = mod.value("amount", 0.0f);
                m.factor = mod.value("factor", 0.0f);
                tmpl.modules.push_back(m);
            }
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
    enemy.m_name = tmpl.name;
    enemy.m_description = tmpl.description;
    enemy.m_texture = tmpl.texture;
    enemy.m_health = tmpl.health;
    enemy.m_currentHealth = tmpl.health;
    enemy.m_speed = tmpl.speed;
    enemy.m_currentSpeed = tmpl.speed;
    enemy.m_reward = tmpl.reward;
    enemy.m_livesOnReach = tmpl.livesOnReach;

    for (auto& mod : tmpl.modules) {
        if (mod.type == "Regeneration")
            enemy.AddModule(std::make_unique<RegenerationModule>(mod.rate));
        else if (mod.type == "Armor")
            enemy.AddModule(std::make_unique<ArmorModule>(mod.amount));
        else if (mod.type == "Resistance")
            enemy.AddModule(std::make_unique<ResistanceModule>(mod.factor));
    }

    return enemy;
}

bool EnemyFactory::Has(const std::string& name) const {
    return m_templates.count(name) > 0;
}
