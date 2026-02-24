#ifndef ENEMY_H
#define ENEMY_H

#include "Entity.h"

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

#endif // ENEMY_H
