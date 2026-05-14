#pragma once

#include <vector>
#include <raylib.h>
#include <world/effect.hpp>
#include <world/enemy.hpp>
#include <lib/dense_slotmap.hpp>

enum class AttackType { Area, Line };

struct Attack {
    Vector2 m_origin;
    std::vector<Vector2> m_targetPositions;
    std::vector<DenseSlotMap<Enemy>::Key> m_targetKeys;
    AttackType m_type;
    float m_radius;
    float m_duration;
    float m_maxDuration;

    float m_damage = 0.0f;
    std::vector<Effect> m_effects;
    float m_delay = 0.0f;
    bool m_resolved = false;

    float Progress() const { return m_duration / m_maxDuration; }
};
