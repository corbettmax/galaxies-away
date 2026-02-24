#ifndef PROJECTILE_H
#define PROJECTILE_H

// ============================================================================
// Projectile Entity
// Represents projectiles fired by player or enemies
// ============================================================================

#include "Entity.h"
#include <vector>

// Forward declarations
class Game;
class Renderer;

class Projectile : public Entity {
public:
    float damage;
    float lifetime;
    float maxLifetime;
    bool isPlayerProjectile;
    bool piercing;
    int pierceCount;
    int maxPierceCount;
    bool homing;
    float homingStrength;
    
    // Trail effect
    std::vector<glm::vec2> trailPositions;
    float trailTimer;
    
    Projectile();
    
    void Update(float deltaTime, Game* game) override;
    void Render(Renderer* renderer) override;
    void OnCollision(Entity* other, Game* game) override;
    
    void SetHoming(float strength);
    void SetPiercing(int count);
    
private:
    void UpdateHoming(float deltaTime, Game* game);
    void UpdateTrail(float deltaTime);
};

#endif // PROJECTILE_H
