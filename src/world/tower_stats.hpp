#pragma once

enum class TargetingMode {
    First,
    Last,
    MostHealth,
    LowestHealth,
    Fastest,
    Slowest,
    MostArmor,
    MostResistance,
    MostShield
};

struct TowerStats {
    float radius         = 0.0f;
    float fireRate       = 0.0f;
    float attackDuration = 0.0f;
    int   targetCount    = 0;
    TargetingMode targetingMode = TargetingMode::First;
};
