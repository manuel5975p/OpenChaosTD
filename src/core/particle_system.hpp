#pragma once

#include <raylib.h>
#include <lib/object_pool.hpp>

struct EmitterDesc {
    Color color = WHITE;
    Color endColor = {255, 255, 255, 0};
    int count = 0;         // 0 = disabled — safe to hold a default desc and emit nothing
    float speed = 50.0f;
    float speedVariance = 20.0f;
    float spread = 360.0f; // degrees; total arc of the spawn cone (360 = full circle)
    float angle = 0.0f;    // degrees; 0=right, 90=down, 180=left, 270=up
    float lifetime = 0.2f;
    float size = 3.0f;
    float endSize = 0.0f;
};

// Internal per-particle runtime state.
struct Particle {
    Vector2 position = {};
    Vector2 velocity = {};
    float lifetime = 0.0f;
    float maxLifetime = 0.0f;
    Color color = WHITE;
    Color endColor = {255, 255, 255, 0};
    float size = 1.0f;
    float endSize = 0.0f;
};

class ParticleSystem {
public:
    ParticleSystem() : m_particles(MAX_PARTICLES) {}

    // Spawn desc.count particles at origin. baseVelocity is added to each particle
    // so effect particles on moving enemies don't lag behind their host.
    void Emit(Vector2 origin, const EmitterDesc& desc, Vector2 baseVelocity = {0, 0});
    void Tick(float dt);
    void Draw() const;
    void Clear();

private:
    static constexpr size_t MAX_PARTICLES = 2048;

    ObjectPool<Particle> m_particles;
};
