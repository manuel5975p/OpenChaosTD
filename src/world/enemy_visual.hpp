#pragma once

#include <string>
#include <engine/features/particle_system.hpp> // EmitterDesc

// The complete visual identity of an enemy. Set once by EnemyFactory, never modified at runtime.
struct EnemyVisual {
    std::string m_texture;                       // resource key for the enemy sprite
    const EmitterDesc* m_deathDescPtr = nullptr; // stable pointer into EmitterPresets; burst emitted on death
};
