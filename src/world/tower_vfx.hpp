#pragma once

#include <world/vfx_effect.hpp>

// Visual identity of a tower's attacks. Set once by TowerFactory, never modified at runtime.
struct TowerVfxDesc {
    VfxStyle style = VfxStyle::Beam;
    Color color = {255, 220, 50, 255};

    EmitterDesc muzzleDesc;         // burst emitted at tower position when it fires
    EmitterDesc trailDesc;          // one particle per tick while VfxEffect is alive
    float trailRate = 0.0f;         // particles/sec; 0 = no trail

    EmitterDesc impactDesc;         // burst at each target on hit
    EmitterDesc critImpactDesc;     // replaces impactDesc on crit when count > 0
};
