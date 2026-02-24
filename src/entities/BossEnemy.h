#ifndef BOSS_ENEMY_H
#define BOSS_ENEMY_H

#include "Enemy.h"

class BossEnemy : public Enemy {
public:
    float specialAttackTimer;
    float specialAttackCooldown;
    int phase;
    
    BossEnemy();
    void UpdateBehavior(float deltaTime, Game* game) override;
    void Render(Renderer* renderer) override;
};

#endif // BOSS_ENEMY_H
