#pragma once

#include <world/vfx_effect.hpp>

// The complete visual identity of a tower's attack. Set once by TowerFactory, never modified at runtime.
struct TowerVisual {
    VfxStyle m_style = VfxStyle::Line;
    Color m_color = {255, 220, 50, 255};
    float m_attackDuration = 0.0f; // how long the beam/ring shows and the muzzle-flash fades

    EmitterDesc m_muzzleDesc;     // burst at the tower when it fires
    EmitterDesc m_impactDesc;     // burst on each enemy hit
    EmitterDesc m_critImpactDesc; // extra burst on a crit hit
};
