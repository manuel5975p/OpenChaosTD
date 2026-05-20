#pragma once

#include <raylib.h>
#include <cstdint>

enum class ParticleDrawMode { Circle, Streak };
enum class EmitterShape { Point, Ring, Disc };

struct EmitterDesc {
    Color color = WHITE;
    Color endColor = {255, 255, 255, 0};
    int count = 0;              // 0 = disabled — safe to hold a default desc and emit nothing
    float speed = 50.0f;
    float speedVariance = 20.0f;
    float spread = 3.14159f;    // half-angle in radians; PI = full circle
    float angle = 0.0f;         // base direction in radians (0=right, PI*1.5=up)
    float lifetime = 0.2f;
    float lifetimeVariance = 0.05f;
    float size = 3.0f;
    float endSize = 0.0f;
    float gravity = 0.0f;       // positive = fall down, negative = rise
    float damping = 0.08f;      // velocity *= pow(damping, dt); lower = quicker stop
    float streakLength = 6.0f;  // used when drawMode == Streak
    ParticleDrawMode drawMode = ParticleDrawMode::Circle;
    EmitterShape shape = EmitterShape::Point;
    float shapeRadius = 0.0f;   // used when shape == Ring or Disc
};

// Per-particle runtime state. Per-emitter constants (gravity, damping,
// streakLength, drawMode) are stored once in ParticleSystem::m_configs
// and referenced here by index to avoid duplicating 16 bytes per particle.
struct Particle {
    Vector2 position = {};
    Vector2 velocity = {};
    float lifetime = 0.0f;
    float maxLifetime = 0.0f;
    Color color = WHITE;
    Color endColor = {255, 255, 255, 0};
    float size = 1.0f;
    float endSize = 0.0f;
    uint8_t configIndex = 0;
};
