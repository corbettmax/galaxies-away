#ifndef TANK_ENEMY_H
#define TANK_ENEMY_H

#include "Enemy.h"

class TankEnemy : public Enemy {
public:
    TankEnemy();
    void UpdateBehavior(float deltaTime, Game* game) override;
    void Render(Renderer* renderer) override;
};

#endif // TANK_ENEMY_H
