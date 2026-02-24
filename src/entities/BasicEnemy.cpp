#include "BasicEnemy.h"
#include "../game.h"

BasicEnemy::BasicEnemy() {
    type = EntityType::ENEMY_BASIC;
    health = maxHealth = 15.0f;
    damage = 10.0f;
    moveSpeed = 100.0f;
    xpValue = 1;
    size = glm::vec2(24.0f);
    radius = 12.0f;
    color = glm::vec4(0.9f, 0.3f, 0.3f, 1.0f);
}

void BasicEnemy::UpdateBehavior(float deltaTime, Game* game) {
    Enemy::MoveTowardsPlayer(deltaTime, game);
}

void BasicEnemy::Render(Renderer* renderer) {
    // Get the enemy texture from the game
    Texture* enemyTex = nullptr;
    if (g_Game) {
        enemyTex = g_Game->GetEnemyTexture();
    }
    
    // Determine facing direction based on velocity
    // Flip horizontally when facing left (negative x velocity)
    bool facingLeft = velocity.x < 0.0f;
    
    // Render the enemy with texture if available, otherwise use solid color
    if (enemyTex && enemyTex->textureID != 0) {
        // Scale up 4x, use white color to preserve original texture colors
        // No rotation (0.0f) to keep level with screen, flip based on direction
        renderer->DrawSpriteWorld(position, size * 4.0f, 0.0f, Colors::WHITE, enemyTex, facingLeft);
    } else {
        // Fallback to solid color rendering
        renderer->DrawSpriteWorld(position, size, rotation, color, nullptr);
        
        // Eye/core
        glm::vec4 coreColor = glm::vec4(1.0f, 0.8f, 0.3f, 1.0f);
        renderer->DrawSpriteWorld(position, size * 0.3f, rotation, coreColor, nullptr);
    }
    
    RenderHealthBar(renderer);
}
