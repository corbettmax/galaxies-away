// ============================================================================
// ParticleSystem Implementation
// ============================================================================

#include "ParticleSystem.h"
#include "../game.h"
#include "../utils.h"
#include <glm/gtc/constants.hpp>
#include <cmath>

ParticleSystem::ParticleSystem() {
    particles.reserve(Constants::MAX_PARTICLES);
    for (int i = 0; i < Constants::MAX_PARTICLES; ++i) {
        particles.push_back(std::make_unique<Particle>());
    }
}

void ParticleSystem::Update(float deltaTime, Game* game) {
    for (auto& particle : particles) {
        if (particle->active) {
            particle->Update(deltaTime, game);
        }
    }
}

void ParticleSystem::Render(Renderer* renderer) {
    for (auto& particle : particles) {
        if (particle->active) {
            particle->Render(renderer);
        }
    }
    renderer->FlushParticles();
}

Particle* ParticleSystem::GetAvailableParticle() {
    for (auto& particle : particles) {
        if (!particle->active) {
            return particle.get();
        }
    }
    return nullptr;
}

void ParticleSystem::SpawnExplosion(const glm::vec2& position, const glm::vec4& color, int count, float speed) {
    for (int i = 0; i < count; ++i) {
        Particle* p = GetAvailableParticle();
        if (!p) break;
        
        p->active = true;
        p->position = position;
        p->velocity = Utils::RandomDirection() * Utils::RandomFloat(speed * 0.3f, speed);
        p->lifetime = 0.0f;
        p->maxLifetime = Utils::RandomFloat(0.3f, 0.8f);
        p->startColor = color;
        p->endColor = glm::vec4(color.r, color.g, color.b, 0.0f);
        p->startSize = Utils::RandomFloat(4.0f, 12.0f);
        p->endSize = 0.0f;
        p->drag = 0.95f;
    }
}

void ParticleSystem::SpawnHitSparks(const glm::vec2& position, const glm::vec2& direction, const glm::vec4& color, int count) {
    for (int i = 0; i < count; ++i) {
        Particle* p = GetAvailableParticle();
        if (!p) break;
        
        p->active = true;
        p->position = position;
        
        glm::vec2 sparkDir = direction;
        if (Utils::Length(sparkDir) < 0.1f) {
            sparkDir = Utils::RandomDirection();
        }
        sparkDir = Utils::Normalize(sparkDir + Utils::RandomDirection() * 0.5f);
        
        p->velocity = sparkDir * Utils::RandomFloat(100.0f, 250.0f);
        p->lifetime = 0.0f;
        p->maxLifetime = Utils::RandomFloat(0.1f, 0.3f);
        p->startColor = color;
        p->endColor = glm::vec4(color.r, color.g, color.b, 0.0f);
        p->startSize = Utils::RandomFloat(2.0f, 6.0f);
        p->endSize = 0.0f;
        p->drag = 0.9f;
    }
}

void ParticleSystem::SpawnTrail(const glm::vec2& position, const glm::vec4& color, float particleSize) {
    Particle* p = GetAvailableParticle();
    if (!p) return;
    
    p->active = true;
    p->position = position + Utils::RandomPointInCircle(3.0f);
    p->velocity = Utils::RandomDirection() * Utils::RandomFloat(10.0f, 30.0f);
    p->lifetime = 0.0f;
    p->maxLifetime = Utils::RandomFloat(0.2f, 0.4f);
    p->startColor = color;
    p->endColor = glm::vec4(color.r, color.g, color.b, 0.0f);
    p->startSize = particleSize;
    p->endSize = 0.0f;
    p->drag = 0.98f;
}

void ParticleSystem::SpawnLevelUp(const glm::vec2& position) {
    // Ring of particles
    int count = 30;
    for (int i = 0; i < count; ++i) {
        Particle* p = GetAvailableParticle();
        if (!p) break;
        
        float angle = (glm::two_pi<float>() / count) * i;
        glm::vec2 dir = glm::vec2(std::cos(angle), std::sin(angle));
        
        p->active = true;
        p->position = position;
        p->velocity = dir * 300.0f;
        p->lifetime = 0.0f;
        p->maxLifetime = 0.5f;
        p->startColor = Colors::YELLOW;
        p->endColor = glm::vec4(1.0f, 1.0f, 0.2f, 0.0f);
        p->startSize = 8.0f;
        p->endSize = 2.0f;
        p->drag = 0.92f;
    }
}

void ParticleSystem::SpawnXPCollect(const glm::vec2& position) {
    for (int i = 0; i < 5; ++i) {
        Particle* p = GetAvailableParticle();
        if (!p) break;
        
        p->active = true;
        p->position = position;
        p->velocity = Utils::RandomDirection() * Utils::RandomFloat(50.0f, 100.0f);
        p->lifetime = 0.0f;
        p->maxLifetime = 0.3f;
        p->startColor = Colors::XP_GREEN;
        p->endColor = glm::vec4(0.4f, 1.0f, 0.4f, 0.0f);
        p->startSize = 4.0f;
        p->endSize = 0.0f;
        p->drag = 0.95f;
    }
}

void ParticleSystem::Clear() {
    for (auto& particle : particles) {
        particle->active = false;
    }
}

int ParticleSystem::GetActiveCount() const {
    int count = 0;
    for (const auto& particle : particles) {
        if (particle->active) count++;
    }
    return count;
}
