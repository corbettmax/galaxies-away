#ifndef PLAYER_H
#define PLAYER_H

#include "Entity.h"
#include "../utils.h"

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

#endif // PLAYER_H
