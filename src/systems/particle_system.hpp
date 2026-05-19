#pragma once

#include <vector>
#include <raylib.h>
#include <world/particle.hpp>

class ParticleSystem {
public:
    // Spawn desc.count particles at origin. baseVelocity is added to each particle
    // so effect particles on moving enemies don't lag behind their host.
    void Emit(Vector2 origin, const EmitterDesc& desc, Vector2 baseVelocity = {0, 0});
    void Tick(float dt);
    void Draw() const;
    void Clear();

private:
    std::vector<Particle> m_particles;
};
