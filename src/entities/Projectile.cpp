// ============================================================================
// Projectile Implementation
// ============================================================================

#include "Projectile.h"
#include "Enemy.h"
#include "Player.h"
#include "../game.h"
#include "../utils.h"

Projectile::Projectile()
    : damage(10.0f)
    , lifetime(0.0f)
    , maxLifetime(3.0f)
    , isPlayerProjectile(true)
    , piercing(false)
    , pierceCount(0)
    , maxPierceCount(1)
    , homing(false)
    , homingStrength(5.0f)
    , trailTimer(0.0f)
{
    type = EntityType::PROJECTILE;
    size = glm::vec2(8.0f);
    radius = 4.0f;
    color = Colors::CYAN;
}

void Projectile::Update(float deltaTime, Game* game) {
    lifetime += deltaTime;
    if (lifetime >= maxLifetime) {
        markedForDeletion = true;
        return;
    }
    
    // Homing behavior
    if (homing && isPlayerProjectile) {
        UpdateHoming(deltaTime, game);
    }
    
    // Update trail
    UpdateTrail(deltaTime);
    
    // Update rotation to face velocity
    if (Utils::Length(velocity) > 0.1f) {
        rotation = Utils::Angle(velocity);
    }
    
    Entity::Update(deltaTime, game);
}

void Projectile::UpdateHoming(float deltaTime, Game* game) {
    if (!game) return;
    
    Enemy* target = game->GetEntityManager()->FindNearestEnemy(position, 300.0f);
    if (!target) return;
    
    glm::vec2 toTarget = Utils::Normalize(target->position - position);
    glm::vec2 currentDir = Utils::Normalize(velocity);
    
    glm::vec2 newDir = Utils::Normalize(currentDir + toTarget * homingStrength * deltaTime);
    float speed = Utils::Length(velocity);
    velocity = newDir * speed;
}

void Projectile::UpdateTrail(float deltaTime) {
    trailTimer += deltaTime;
    if (trailTimer >= 0.02f) {
        trailTimer = 0.0f;
        trailPositions.insert(trailPositions.begin(), position);
        if (trailPositions.size() > 10) {
            trailPositions.pop_back();
        }
    }
}

void Projectile::Render(Renderer* renderer) {
    // Render trail
    for (size_t i = 0; i < trailPositions.size(); ++i) {
        float alpha = 1.0f - (float)i / trailPositions.size();
        float trailSize = size.x * (1.0f - (float)i / trailPositions.size() * 0.5f);
        glm::vec4 trailColor = color;
        trailColor.a *= alpha * 0.5f;
        renderer->DrawSpriteWorld(trailPositions[i], glm::vec2(trailSize), rotation, trailColor, nullptr);
    }
    
    // Render projectile
    renderer->DrawSpriteWorld(position, size, rotation, color, nullptr);
    
    // Bright core
    glm::vec4 coreColor = Colors::WHITE;
    coreColor.a = 0.8f;
    renderer->DrawSpriteWorld(position, size * 0.5f, rotation, coreColor, nullptr);
}

void Projectile::OnCollision(Entity* other, Game* game) {
    if (isPlayerProjectile && (other->type == EntityType::ENEMY_BASIC ||
                               other->type == EntityType::ENEMY_TANK ||
                               other->type == EntityType::ENEMY_FAST ||
                               other->type == EntityType::ENEMY_BOSS)) {
        Enemy* enemy = static_cast<Enemy*>(other);
        enemy->TakeDamage(damage, game);
        
        if (piercing) {
            pierceCount++;
            if (pierceCount >= maxPierceCount) {
                markedForDeletion = true;
            }
        } else {
            markedForDeletion = true;
        }
    } else if (!isPlayerProjectile && other->type == EntityType::PLAYER) {
        Player* player = static_cast<Player*>(other);
        player->TakeDamage(damage, game);
        markedForDeletion = true;
    }
}

void Projectile::SetHoming(float strength) {
    homing = true;
    homingStrength = strength;
}

void Projectile::SetPiercing(int count) {
    piercing = true;
    maxPierceCount = count;
    pierceCount = 0;
}
