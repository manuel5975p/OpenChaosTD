#pragma once

#include <vector>
#include <memory>
#include <raylib.h>
#include <string>

#include <world/tower_modules.hpp>
#include <world/attack.hpp>

enum class TargetingMode {
    First,
    Last,
    MostHealth,
    LowestHealth,
    Fastest,
    Slowest
};


class Tower{
public:
    std::string m_name;
    std::string m_texture;
    Vector2 m_position;

    int m_targetCount = 1;
    float m_radius = 0.0f;
    float m_fireRate = 1.0f;
    float m_attackDuration = 0.15f;
    float m_cooldown = 0.0f;
    float m_attackFlash = 0.0f;
    AttackType m_attackType = AttackType::Line;
    TargetingMode m_targetingMode = TargetingMode::First;
    std::vector<DenseSlotMap<Enemy>::Key> m_currentTargetKeys;
    std::vector<std::unique_ptr<TowerModule>> m_modules;

    void AddModule(std::unique_ptr<TowerModule> mod) {
        m_modules.push_back(std::move(mod));
    }
};