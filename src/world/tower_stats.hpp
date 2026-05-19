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
    float fireRate       = 1.0f;
    float attackDuration = 0.15f;
    int   targetCount    = 1;
    TargetingMode targetingMode = TargetingMode::First;
};
