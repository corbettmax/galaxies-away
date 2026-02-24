#ifndef ENTITYMANAGER_H
#define ENTITYMANAGER_H

// ============================================================================
// Entity Manager
// Central manager for all entities in the game
// ============================================================================

#include "Player.h"
#include "Enemy.h"
#include "Projectile.h"
#include "XPOrb.h"
#include "ParticleSystem.h"
#include <vector>
#include <memory>

// Forward declarations
class Game;
class Renderer;

class EntityManager {
public:
    std::unique_ptr<Player> player;
    std::vector<std::unique_ptr<Enemy>> enemies;
    std::vector<std::unique_ptr<Projectile>> projectiles;
    std::vector<std::unique_ptr<XPOrb>> xpOrbs;
    ParticleSystem particleSystem;
    
    EntityManager();
    
    void Update(float deltaTime, Game* game);
    void Render(Renderer* renderer);
    
    // Entity spawning
    void SpawnPlayer(const glm::vec2& position);
    Enemy* SpawnEnemy(EntityType type, const glm::vec2& position);
    Projectile* SpawnProjectile(const glm::vec2& position, const glm::vec2& velocity, 
                                float damage, bool isPlayerProjectile);
    void SpawnXPOrb(const glm::vec2& position, int value);
    
    // Queries
    Enemy* FindNearestEnemy(const glm::vec2& position, float maxRange = -1.0f);
    std::vector<Enemy*> FindEnemiesInRange(const glm::vec2& position, float range);
    int GetEnemyCount() const;
    int GetProjectileCount() const;
    
    // Cleanup
    void CleanupDeadEntities();
    void Clear();
    
private:
    void UpdateCollisions(Game* game);
    void CheckProjectileEnemyCollisions(Game* game);
    void CheckPlayerEnemyCollisions(Game* game);
    void CheckPlayerXPCollisions(Game* game);
};

#endif // ENTITYMANAGER_H
