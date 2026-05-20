#pragma once

#include <raylib.h>
#include <world/particle.hpp>
#include <lib/object_pool.hpp>

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
    // Per-emitter constants shared across all particles of the same type.
    // Stored once and referenced by Particle::configIndex.
    struct ParticleConfig {
        float gravity = 0.0f;
        float damping = 0.08f;
        float streakLength = 6.0f;
        ParticleDrawMode drawMode = ParticleDrawMode::Circle;
    };

    static constexpr size_t MAX_PARTICLES = 2048;
    static constexpr size_t MAX_CONFIGS   = 32;

    // Finds an existing matching config or inserts a new one.
    // Returns the config index. Falls back to 0 if the table is full.
    uint8_t FindOrAddConfig(const ParticleConfig& cfg);

    ObjectPool<Particle> m_particles;
    ParticleConfig m_configs[MAX_CONFIGS];
    uint8_t m_configCount = 0;
};
