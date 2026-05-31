#pragma once

#include <raylib.h>
#include <vector>
#include <engine/features/particle_system.hpp>

enum class VfxStyle { Line, Ring };

struct VfxEffect {
    Vector2 m_origin;
    std::vector<Vector2> m_targetPositions;
    VfxStyle m_style = VfxStyle::Line;
    Color m_color = {255, 220, 50, 255};
    float m_radius = 0.0f;
    float m_duration = 0.0f;
    float m_maxDuration = 0.0f;

    EmitterDesc m_trailDesc;
    float m_trailAccumulator = 0.0f;

    float Progress() const {
        return (m_maxDuration > 0.0f) ? m_duration / m_maxDuration : 0.0f;
    }
};
