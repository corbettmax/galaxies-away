#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

// ============================================================================
// Particle System
// Manages all particle effects in the game
// ============================================================================

#include "Particle.h"
#include <vector>
#include <memory>

// Forward declarations
class Game;
class Renderer;

class ParticleSystem {
public:
    std::vector<std::unique_ptr<Particle>> particles;
    
    ParticleSystem();
    
    void Update(float deltaTime, Game* game);
    void Render(Renderer* renderer);
    
    // Particle emitters
    void SpawnExplosion(const glm::vec2& position, const glm::vec4& color, int count = 20, float speed = 200.0f);
    void SpawnHitSparks(const glm::vec2& position, const glm::vec2& direction, const glm::vec4& color, int count = 5);
    void SpawnTrail(const glm::vec2& position, const glm::vec4& color, float size = 4.0f);
    void SpawnLevelUp(const glm::vec2& position);
    void SpawnXPCollect(const glm::vec2& position);
    
    void Clear();
    int GetActiveCount() const;
    
private:
    Particle* GetAvailableParticle();
};

#endif // PARTICLESYSTEM_H
