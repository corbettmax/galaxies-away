#ifndef FAST_ENEMY_H
#define FAST_ENEMY_H

#include "Enemy.h"

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

#endif // FAST_ENEMY_H
