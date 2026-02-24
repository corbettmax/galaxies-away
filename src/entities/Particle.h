#ifndef PARTICLE_H
#define PARTICLE_H

// ============================================================================
// Particle Entity
// Visual effects particle for explosions, trails, and other effects
// ============================================================================

#include "Entity.h"

// Forward declarations
class Game;
class Renderer;

class Particle : public Entity {
public:
    float lifetime;
    float maxLifetime;
    glm::vec4 startColor;
    glm::vec4 endColor;
    float startSize;
    float endSize;
    float drag;
    
    Particle();
    
    void Update(float deltaTime, Game* game) override;
    void Render(Renderer* renderer) override;
};

#endif // PARTICLE_H
