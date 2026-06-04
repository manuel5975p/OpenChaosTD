#pragma once

#include <raylib.h>
#include <engine/lib/object_pool.hpp>

struct EmitterDesc {
    Color m_color = WHITE;
    Color m_endColor = {255, 255, 255, 0};
    int m_count = 0;         // 0 = disabled — safe to hold a default desc and emit nothing
    float m_speed = 50.0f;
    float m_speedVariance = 20.0f;
    float m_spread = 360.0f; // degrees; total arc of the spawn cone (360 = full circle)
    float m_angle = 0.0f;    // degrees; 0=right, 90=down, 180=left, 270=up
    float m_lifetime = 0.2f;
    float m_size = 3.0f;
    float m_endSize = 0.0f;
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
    ParticleSystem() : m_particles(kMaxParticles) {}

    // Spawn desc.m_count particles at origin. baseVelocity is added to each particle
    // so effect particles on moving enemies don't lag behind their host.
    void Emit(Vector2 origin, const EmitterDesc& desc, Vector2 baseVelocity = {0, 0});
    void Tick(float dt);
    void Draw() const;
    void Clear();

private:
    static constexpr size_t kMaxParticles = 2048;

    ObjectPool<Particle> m_particles;
};
