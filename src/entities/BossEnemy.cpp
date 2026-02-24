#include "BossEnemy.h"
#include "../game.h"

BossEnemy::BossEnemy()
    : specialAttackTimer(0.0f)
    , specialAttackCooldown(5.0f)
    , phase(1)
{
    type = EntityType::ENEMY_BOSS;
    health = maxHealth = 500.0f;
    damage = 30.0f;
    moveSpeed = 60.0f;
    xpValue = 50;
    size = glm::vec2(80.0f);
    radius = 40.0f;
    color = glm::vec4(0.5f, 0.1f, 0.5f, 1.0f);
}

void BossEnemy::UpdateBehavior(float deltaTime, Game* game) {
    if (!game || !game->GetEntityManager()->player) return;
    
    // Update phase based on health
    if (health < maxHealth * 0.3f) {
        phase = 3;
        moveSpeed = 100.0f;
    } else if (health < maxHealth * 0.6f) {
        phase = 2;
        moveSpeed = 80.0f;
    }
    
    // Special attack
    specialAttackTimer -= deltaTime;
    if (specialAttackTimer <= 0.0f) {
        specialAttackTimer = specialAttackCooldown / phase;
        
        // Spawn projectiles in a ring
        int projectileCount = 8 * phase;
        for (int i = 0; i < projectileCount; ++i) {
            float angle = (glm::two_pi<float>() / projectileCount) * i;
            glm::vec2 dir = glm::vec2(std::cos(angle), std::sin(angle));
            
            Projectile* proj = game->GetEntityManager()->SpawnProjectile(
                position + dir * radius,
                dir * 150.0f,
                15.0f,
                false
            );
            proj->color = Colors::PURPLE;
            proj->maxLifetime = 3.0f;
        }
    }
    
    // Chase player
    Enemy::MoveTowardsPlayer(deltaTime, game);
}

void BossEnemy::Render(Renderer* renderer) {
    // Large boss body
    renderer->DrawSpriteWorld(position, size, rotation, color, nullptr);
    
    // Outer ring
    float pulsePhase = static_cast<float>(glfwGetTime()) * 2.0f;
    float pulse = 0.9f + 0.1f * std::sin(pulsePhase);
    glm::vec4 ringColor = glm::vec4(0.7f, 0.2f, 0.7f, 0.6f);
    renderer->DrawSpriteWorld(position, size * 1.2f * pulse, rotation + pulsePhase * 0.5f, ringColor, nullptr);
    
    // Inner core
    glm::vec4 coreColor = glm::vec4(1.0f, 0.3f, 1.0f, 1.0f);
    renderer->DrawSpriteWorld(position, size * 0.4f, -rotation * 2.0f, coreColor, nullptr);
    
    // Phase indicator
    for (int i = 0; i < phase; ++i) {
        float orbitAngle = pulsePhase + (glm::two_pi<float>() / 3.0f) * i;
        glm::vec2 orbitPos = position + glm::vec2(std::cos(orbitAngle), std::sin(orbitAngle)) * size.x * 0.6f;
        renderer->DrawSpriteWorld(orbitPos, glm::vec2(10.0f), 0.0f, Colors::MAGENTA, nullptr);
    }
    
    RenderHealthBar(renderer);
}
