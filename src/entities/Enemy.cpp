#include "Enemy.h"
#include "Player.h"
#include "../game.h"

Enemy::Enemy()
    : health(10.0f)
    , maxHealth(10.0f)
    , damage(10.0f)
    , moveSpeed(80.0f)
    , xpValue(1)
    , attackCooldown(1.0f)
    , attackTimer(0.0f)
{
    type = EntityType::ENEMY_BASIC;
    color = Colors::ENEMY_RED;
}

void Enemy::Update(float deltaTime, Game* game) {
    // Update attack timer
    if (attackTimer > 0.0f) {
        attackTimer -= deltaTime;
    }
    
    // Update behavior (overridden by subclasses)
    UpdateBehavior(deltaTime, game);
    
    Entity::Update(deltaTime, game);
}

void Enemy::Render(Renderer* renderer) {
    Entity::Render(renderer);
    RenderHealthBar(renderer);
}

void Enemy::OnCollision(Entity* other, Game* game) {
    if (other->type == EntityType::PLAYER) {
        Player* player = static_cast<Player*>(other);
        if (attackTimer <= 0.0f) {
            player->TakeDamage(damage, game);
            attackTimer = attackCooldown;
        }
    }
}

void Enemy::UpdateBehavior(float deltaTime, Game* game) {
    MoveTowardsPlayer(deltaTime, game);
}

void Enemy::TakeDamage(float dmg, Game* game) {
    health -= dmg;
    
    // Flash effect could be added here
    
    if (health <= 0.0f) {
        markedForDeletion = true;
        
        // Spawn XP orb
        if (game) {
            game->GetEntityManager()->SpawnXPOrb(position, xpValue);
            game->GetEntityManager()->particleSystem.SpawnExplosion(position, color, 15, 150.0f);
        }
    } else {
        // Hit sparks
        if (game) {
            game->GetEntityManager()->particleSystem.SpawnHitSparks(
                position, glm::vec2(0.0f), Colors::ORANGE, 3);
        }
    }
}

void Enemy::MoveTowardsPlayer(float deltaTime, Game* game) {
    (void)deltaTime; // Suppress unused warning
    if (!game || !game->GetEntityManager()->player) return;
    
    Player* player = game->GetEntityManager()->player.get();
    glm::vec2 direction = Utils::Normalize(player->position - position);
    
    velocity = direction * moveSpeed;
    
    // Face the player
    rotation = Utils::Angle(direction) + glm::half_pi<float>();
}

void Enemy::RenderHealthBar(Renderer* renderer) {
    if (health >= maxHealth) return;
    
    float barWidth = size.x * 1.2f;
    float barHeight = 4.0f;
    float yOffset = size.y * 0.6f + 5.0f;
    
    glm::vec2 screenPos = renderer->WorldToScreen(position);
    glm::vec2 barPos = screenPos - glm::vec2(barWidth * 0.5f, yOffset);
    
    float healthPercent = health / maxHealth;
    
    // Background
    renderer->DrawRect(barPos, glm::vec2(barWidth, barHeight), glm::vec4(0.2f, 0.2f, 0.2f, 0.8f));
    
    // Health fill
    glm::vec4 healthColor = Utils::Lerp(Colors::RED, Colors::GREEN, healthPercent);
    renderer->DrawRect(barPos, glm::vec2(barWidth * healthPercent, barHeight), healthColor);
}
