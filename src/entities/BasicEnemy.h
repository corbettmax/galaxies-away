#ifndef BASIC_ENEMY_H
#define BASIC_ENEMY_H

#include "Enemy.h"

class BasicEnemy : public Enemy {
public:
    BasicEnemy();
    void UpdateBehavior(float deltaTime, Game* game) override;
    void Render(Renderer* renderer) override;
};

#endif // BASIC_ENEMY_H
