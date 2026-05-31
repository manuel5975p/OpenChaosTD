#pragma once

#include <world/vfx_effect.hpp>

// Visual identity of a tower's attacks. Set once by TowerFactory, never modified at runtime.
struct TowerVfxDesc {
    VfxStyle style = VfxStyle::Line;
    Color color = {255, 220, 50, 255};

    EmitterDesc muzzleDesc;         // burst emitted at tower position when it fires

};
