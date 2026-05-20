#include <systems/particle_system.hpp>

#include <raymath.h>
#include <cmath>
#include <iostream>

uint8_t ParticleSystem::FindOrAddConfig(const ParticleConfig& cfg) {
    // Linear scan — table is tiny (at most MAX_CONFIGS distinct emitter types)
    for (uint8_t i = 0; i < m_configCount; i++) {
        auto& c = m_configs[i];
        if (c.gravity == cfg.gravity && c.damping == cfg.damping
            && c.streakLength == cfg.streakLength && c.drawMode == cfg.drawMode)
            return i;
    }
    if (m_configCount >= MAX_CONFIGS) {
        std::cerr << "ParticleSystem: config table full, falling back to slot 0\n";
        return 0;
    }
    m_configs[m_configCount] = cfg;
    return m_configCount++;
}

void ParticleSystem::Emit(Vector2 origin, const EmitterDesc& desc, Vector2 baseVelocity) {
    if (desc.count <= 0) return;

    // Register the per-emitter constants once for this burst
    uint8_t cfgIdx = FindOrAddConfig({desc.gravity, desc.damping, desc.streakLength, desc.drawMode});

    for (int i = 0; i < desc.count; i++) {
        Particle* p = m_particles.Acquire();
        if (!p) break; // pool at capacity — drop remaining particles silently

        // Resolve spawn position from emitter shape
        p->position = origin;
        if (desc.shape == EmitterShape::Ring || desc.shape == EmitterShape::Disc) {
            float a = (float)GetRandomValue(0, 62831) / 10000.0f;
            float r = desc.shapeRadius;
            if (desc.shape == EmitterShape::Disc)
                r *= std::sqrtf((float)GetRandomValue(0, 10000) / 10000.0f);
            p->position = {origin.x + std::cosf(a) * r, origin.y + std::sinf(a) * r};
        }

        // Randomise direction within spread cone around base angle
        float dirAngle = desc.angle + ((float)GetRandomValue(-10000, 10000) / 10000.0f) * desc.spread;
        float speed    = desc.speed + ((float)GetRandomValue(-10000, 10000) / 10000.0f) * desc.speedVariance;
        speed = std::max(0.0f, speed);

        float life = desc.lifetime + ((float)GetRandomValue(-10000, 10000) / 10000.0f) * desc.lifetimeVariance;
        life = std::max(0.01f, life);

        p->velocity    = {std::cosf(dirAngle) * speed + baseVelocity.x,
                          std::sinf(dirAngle) * speed + baseVelocity.y};
        p->lifetime    = life;
        p->maxLifetime = life;
        p->color       = desc.color;
        p->endColor    = desc.endColor;
        p->size        = desc.size;
        p->endSize     = desc.endSize;
        p->configIndex = cfgIdx;
    }
}

void ParticleSystem::Tick(float dt) {
    size_t i = 0;
    while (i < m_particles.Size()) {
        Particle& p = m_particles[i];
        const ParticleConfig& cfg = m_configs[p.configIndex];

        p.velocity.y += cfg.gravity * dt;
        p.velocity = Vector2Scale(p.velocity, std::powf(cfg.damping, dt));
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
        const ParticleConfig& cfg = m_configs[p.configIndex];
        float t = p.lifetime / p.maxLifetime;
        Color c = ColorLerp(p.endColor, p.color, t);
        float s = Lerp(p.endSize, p.size, t);

        if (cfg.drawMode == ParticleDrawMode::Streak) {
            float speed = Vector2Length(p.velocity);
            if (speed > 0.1f) {
                Vector2 norm = Vector2Normalize(p.velocity);
                Vector2 tail = Vector2Subtract(p.position, Vector2Scale(norm, cfg.streakLength * t));
                DrawLineEx(tail, p.position, std::max(s, 1.0f), c);
                continue;
            }
        }
        DrawCircleV(p.position, s, c);
    }
}

void ParticleSystem::Clear() {
    m_particles.Clear();
    m_configCount = 0;
}
