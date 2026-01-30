#ifndef ENTITIES_H
#define ENTITIES_H

// ============================================================================
// Galaxies Away - Entity System
// Defines all game entities: Player, Enemies, Projectiles, XP Orbs, Particles
// ============================================================================

#include "utils.h"
#include "renderer.h"

// Forward declaration
class Game;

// ============================================================================
// Base Entity Class
// ============================================================================

class Entity {
public:
    glm::vec2 position;
    glm::vec2 velocity;
    glm::vec2 size;
    float rotation;
    float radius;           // Collision radius
    glm::vec4 color;
    EntityType type;
    bool active;
    bool markedForDeletion;
    
    Entity();
    virtual ~Entity() = default;
    
    virtual void Update(float deltaTime, Game* game);
    virtual void Render(Renderer* renderer);
    virtual void OnCollision(Entity* other, Game* game);
    
    bool CollidesWith(Entity* other) const;
    float DistanceTo(Entity* other) const;
    float DistanceTo(const glm::vec2& point) const;
};

// ============================================================================
// Player Class
// ============================================================================

class Player : public Entity {
public:
    // Stats
    float health;
    float maxHealth;
    float moveSpeed;
    float pickupRadius;
    
    // Experience and leveling
    int experience;
    int level;
    int experienceToNextLevel;
    
    // Combat stats
    float damageMultiplier;
    float fireRateMultiplier;
    int projectileCountBonus;
    float projectileSizeMultiplier;
    
    // Movement input
    glm::vec2 moveInput;
    
    // Invincibility frames
    float invincibilityTimer;
    float invincibilityDuration;
    
    // Visual
    float engineGlow;
    
    Player();
    
    void Update(float deltaTime, Game* game) override;
    void Render(Renderer* renderer) override;
    void OnCollision(Entity* other, Game* game) override;
    
    void TakeDamage(float damage, Game* game);
    void AddExperience(int amount, Game* game);
    void ApplyUpgrade(const UpgradeChoice& upgrade);
    void Heal(float amount);
    
    int GetExperienceForLevel(int lvl) const;
    bool IsInvincible() const { return invincibilityTimer > 0.0f; }
    
private:
    void UpdateMovement(float deltaTime);
    void RenderShip(Renderer* renderer);
    void RenderEngineTrail(Renderer* renderer);
};

// ============================================================================
// Enemy Base Class
// ============================================================================

class Enemy : public Entity {
public:
    float health;
    float maxHealth;
    float damage;
    float moveSpeed;
    int xpValue;
    float attackCooldown;
    float attackTimer;
    
    Enemy();
    
    void Update(float deltaTime, Game* game) override;
    void Render(Renderer* renderer) override;
    void OnCollision(Entity* other, Game* game) override;
    
    virtual void UpdateBehavior(float deltaTime, Game* game);
    void TakeDamage(float damage, Game* game);
    
protected:
    void MoveTowardsPlayer(float deltaTime, Game* game);
    void RenderHealthBar(Renderer* renderer);
};

// ============================================================================
// Enemy Types
// ============================================================================

// Basic enemy - simple chaser
class BasicEnemy : public Enemy {
public:
    BasicEnemy();
    void UpdateBehavior(float deltaTime, Game* game) override;
    void Render(Renderer* renderer) override;
};

// Tank enemy - slow but high HP
class TankEnemy : public Enemy {
public:
    TankEnemy();
    void UpdateBehavior(float deltaTime, Game* game) override;
    void Render(Renderer* renderer) override;
};

// Fast enemy - quick but low HP, sometimes dodges
class FastEnemy : public Enemy {
public:
    float dodgeTimer;
    float dodgeCooldown;
    glm::vec2 dodgeDirection;
    bool isDodging;
    
    FastEnemy();
    void UpdateBehavior(float deltaTime, Game* game) override;
    void Render(Renderer* renderer) override;
};

// Boss enemy - large, high HP, spawns periodically
class BossEnemy : public Enemy {
public:
    float specialAttackTimer;
    float specialAttackCooldown;
    int phase;
    
    BossEnemy();
    void UpdateBehavior(float deltaTime, Game* game) override;
    void Render(Renderer* renderer) override;
};

// ============================================================================
// Projectile Class
// ============================================================================

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

// ============================================================================
// XP Orb Class
// ============================================================================

class XPOrb : public Entity {
public:
    int xpValue;
    float magnetSpeed;
    bool beingCollected;
    float lifetime;
    float pulseTimer;
    
    XPOrb(int value = 1);
    
    void Update(float deltaTime, Game* game) override;
    void Render(Renderer* renderer) override;
};

// ============================================================================
// Particle Class
// ============================================================================

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

// ============================================================================
// Particle System
// ============================================================================

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

// ============================================================================
// Entity Manager
// ============================================================================

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

#endif // ENTITIES_H
