#pragma once

#include <world/vfx_effect.hpp>

// The complete visual identity of a tower's attack. Set once by TowerFactory, never modified at runtime.
struct TowerVisual {
    VfxStyle style = VfxStyle::Line;
    Color color = {255, 220, 50, 255};
    float attackDuration = 0.0f; // how long the beam/ring shows and the muzzle-flash fades

    EmitterDesc muzzleDesc;     // burst at the tower when it fires
    EmitterDesc impactDesc;     // burst on each enemy hit
    EmitterDesc critImpactDesc; // extra burst on a crit hit
};
