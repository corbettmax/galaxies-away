#include "TankEnemy.h"
#include "../game.h"

TankEnemy::TankEnemy() {
    type = EntityType::ENEMY_TANK;
    health = maxHealth = 60.0f;
    damage = 20.0f;
    moveSpeed = 50.0f;
    xpValue = 5;
    size = glm::vec2(48.0f);
    radius = 24.0f;
    color = glm::vec4(0.6f, 0.2f, 0.2f, 1.0f);
}

void TankEnemy::UpdateBehavior(float deltaTime, Game* game) {
    Enemy::MoveTowardsPlayer(deltaTime, game);
}

void TankEnemy::Render(Renderer* renderer) {
    // Large armored enemy
    renderer->DrawSpriteWorld(position, size, rotation, color, nullptr);
    
    // Armor plates
    glm::vec4 armorColor = glm::vec4(0.4f, 0.15f, 0.15f, 1.0f);
    renderer->DrawSpriteWorld(position, size * 0.7f, rotation + 0.785f, armorColor, nullptr);
    
    // Core
    glm::vec4 coreColor = glm::vec4(1.0f, 0.4f, 0.2f, 1.0f);
    renderer->DrawSpriteWorld(position, size * 0.25f, rotation, coreColor, nullptr);
    
    RenderHealthBar(renderer);
}
