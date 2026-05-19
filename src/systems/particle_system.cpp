#include <systems/particle_system.hpp>

#include <raymath.h>
#include <cmath>
#include <algorithm>

void ParticleSystem::Emit(Vector2 origin, const EmitterDesc& desc, Vector2 baseVelocity) {
    if (desc.count <= 0) return;

    for (int i = 0; i < desc.count; i++) {
        // Resolve spawn position from emitter shape
        Vector2 spawnPos = origin;
        if (desc.shape == EmitterShape::Ring || desc.shape == EmitterShape::Disc) {
            float a = (float)GetRandomValue(0, 62831) / 10000.0f;
            float r = desc.shapeRadius;
            if (desc.shape == EmitterShape::Disc)
                r *= std::sqrtf((float)GetRandomValue(0, 10000) / 10000.0f);
            spawnPos = {origin.x + std::cosf(a) * r, origin.y + std::sinf(a) * r};
        }

        // Randomise direction within spread cone around base angle
        float halfSpread = desc.spread;
        float dirAngle = desc.angle + ((float)GetRandomValue(-10000, 10000) / 10000.0f) * halfSpread;
        float speed = desc.speed + ((float)GetRandomValue(-10000, 10000) / 10000.0f) * desc.speedVariance;
        speed = std::max(0.0f, speed);

        float life = desc.lifetime + ((float)GetRandomValue(-10000, 10000) / 10000.0f) * desc.lifetimeVariance;
        life = std::max(0.01f, life);

        Particle p;
        p.position = spawnPos;
        p.velocity = {std::cosf(dirAngle) * speed + baseVelocity.x,
                      std::sinf(dirAngle) * speed + baseVelocity.y};
        p.lifetime = life;
        p.maxLifetime = life;
        p.color = desc.color;
        p.endColor = desc.endColor;
        p.size = desc.size;
        p.endSize = desc.endSize;
        p.gravity = desc.gravity;
        p.damping = desc.damping;
        p.streakLength = desc.streakLength;
        p.drawMode = desc.drawMode;
        m_particles.push_back(p);
    }
}

void ParticleSystem::Tick(float dt) {
    for (auto& p : m_particles) {
        p.velocity.y += p.gravity * dt;
        p.velocity = Vector2Scale(p.velocity, std::powf(p.damping, dt));
        p.position = Vector2Add(p.position, Vector2Scale(p.velocity, dt));
        p.lifetime -= dt;
    }
    std::erase_if(m_particles, [](const Particle& p) { return p.lifetime <= 0.0f; });
}

void ParticleSystem::Draw() const {
    for (const auto& p : m_particles) {
        float t = p.lifetime / p.maxLifetime;
        Color c = ColorLerp(p.endColor, p.color, t);
        float s = Lerp(p.endSize, p.size, t);

        if (p.drawMode == ParticleDrawMode::Streak) {
            float speed = Vector2Length(p.velocity);
            if (speed > 0.1f) {
                Vector2 norm = Vector2Normalize(p.velocity);
                Vector2 tail = Vector2Subtract(p.position, Vector2Scale(norm, p.streakLength * t));
                DrawLineEx(tail, p.position, std::max(s, 1.0f), c);
                continue;
            }
        }
        DrawCircleV(p.position, s, c);
    }
}

void ParticleSystem::Clear() {
    m_particles.clear();
}
