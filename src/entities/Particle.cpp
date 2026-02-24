// ============================================================================
// Particle Implementation
// ============================================================================

#include "Particle.h"
#include "../game.h"
#include "../utils.h"
#include <cmath>

Particle::Particle()
    : lifetime(0.0f)
    , maxLifetime(1.0f)
    , startColor(Colors::WHITE)
    , endColor(glm::vec4(1.0f, 1.0f, 1.0f, 0.0f))
    , startSize(8.0f)
    , endSize(0.0f)
    , drag(0.98f)
{
    type = EntityType::PARTICLE;
    active = false;
}

void Particle::Update(float deltaTime, Game* game) {
    if (!active) return;
    
    lifetime += deltaTime;
    if (lifetime >= maxLifetime) {
        active = false;
        return;
    }
    
    // Apply drag
    velocity *= std::pow(drag, deltaTime * 60.0f);
    
    // Update visual properties
    float t = lifetime / maxLifetime;
    color = Utils::Lerp(startColor, endColor, t);
    float currentSize = Utils::Lerp(startSize, endSize, t);
    size = glm::vec2(currentSize);
    
    Entity::Update(deltaTime, game);
}

void Particle::Render(Renderer* renderer) {
    if (!active) return;
    renderer->DrawParticle(position, size.x, color);
}
