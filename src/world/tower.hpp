#pragma once

#include <vector>
#include <memory>
#include <raylib.h>
#include <string>

#include <world/tower_modules.hpp>

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
    Vector2 m_position;

    int m_targetCount;
    float m_radius;
    float m_fireRate;
    float m_cooldown;
    TargetingMode m_targetingMode;
    std::vector<std::unique_ptr<TowerModule>> m_modules;

    void AddModule(std::unique_ptr<TowerModule> mod) {
        m_modules.push_back(std::move(mod));
    }
};