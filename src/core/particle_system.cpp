#include <core/particle_system.hpp>

#include <raymath.h>

void ParticleSystem::Emit(Vector2 origin, const EmitterDesc& desc, Vector2 baseVelocity) {
    if (desc.count <= 0) return;

    // Convert once per burst — angle and spread are per-emitter, not per-particle
    float baseAngle = desc.angle * DEG2RAD;
    float halfArc   = desc.spread * 0.5f * DEG2RAD;

    for (int i = 0; i < desc.count; i++) {
        Particle* p = m_particles.Acquire();
        if (!p) break; // pool at capacity — drop remaining particles silently

        p->position = origin;

        // Randomise direction within spread cone around base angle
        float dirAngle = baseAngle + ((float)GetRandomValue(-10000, 10000) / 10000.0f) * halfArc;
        float speed    = desc.speed + ((float)GetRandomValue(-10000, 10000) / 10000.0f) * desc.speedVariance;
        speed = std::max(0.0f, speed);

        float life = std::max(0.01f, desc.lifetime);

        p->velocity = {std::cosf(dirAngle) * speed + baseVelocity.x, std::sinf(dirAngle) * speed + baseVelocity.y};
        p->lifetime = life;
        p->maxLifetime = life;
        p->color = desc.color;
        p->endColor = desc.endColor;
        p->size = desc.size;
        p->endSize = desc.endSize;

    }
}

void ParticleSystem::Tick(float dt) {
    size_t i = 0;
    while (i < m_particles.Size()) {
        Particle& p = m_particles[i];

        p.position = Vector2Add(p.position, Vector2Scale(p.velocity, dt));
        p.lifetime -= dt;

        if (p.lifetime <= 0.0f) {
            m_particles.ReleaseAt(i);
            // i unchanged — the former last particle is now at this index
        } else {
            i++;
        }
    }
}

void ParticleSystem::Draw() const {
    for (const auto& p : m_particles) {
        float t = p.lifetime / p.maxLifetime;
        Color c = ColorLerp(p.endColor, p.color, t);
        float s = Lerp(p.endSize, p.size, t);

        DrawCircleV(p.position, s, c);
    }
}

void ParticleSystem::Clear() {
    m_particles.Clear();
}
