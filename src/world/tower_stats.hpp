#pragma once

#include <world/attack.hpp>

enum class TargetingMode {
    First,
    Last,
    MostHealth,
    LowestHealth,
    Fastest,
    Slowest
};

struct TowerStats {
    float radius         = 0.0f;
    float fireRate       = 1.0f;
    float attackDuration = 0.15f;
    int   targetCount    = 1;
    AttackType    attackType    = AttackType::Line;
    TargetingMode targetingMode = TargetingMode::First;
};
