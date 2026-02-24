#include "FastEnemy.h"
#include "Player.h"
#include "../game.h"

FastEnemy::FastEnemy()
    : dodgeTimer(0.0f)
    , dodgeCooldown(2.0f)
    , dodgeDirection(0.0f)
    , isDodging(false)
{
    type = EntityType::ENEMY_FAST;
    health = maxHealth = 8.0f;
    damage = 8.0f;
    moveSpeed = 200.0f;
    xpValue = 2;
    size = glm::vec2(20.0f);
    radius = 10.0f;
    color = glm::vec4(1.0f, 0.5f, 0.8f, 1.0f);
}

void FastEnemy::UpdateBehavior(float deltaTime, Game* game) {
    if (!game || !game->GetEntityManager()->player) return;
    
    Player* player = game->GetEntityManager()->player.get();
    
    // Dodge timer
    if (dodgeTimer > 0.0f) {
        dodgeTimer -= deltaTime;
        
        if (isDodging) {
            velocity = dodgeDirection * moveSpeed * 2.0f;
            if (dodgeTimer <= 0.0f) {
                isDodging = false;
            }
            return;
        }
    }
    
    // Chance to dodge when projectile is nearby
    if (dodgeTimer <= 0.0f) {
        auto& projectiles = game->GetEntityManager()->projectiles;
        for (auto& proj : projectiles) {
            if (!proj->isPlayerProjectile) continue;
            
            float dist = DistanceTo(proj.get());
            if (dist < 100.0f && Utils::RandomFloat(0.0f, 1.0f) < 0.3f) {
                // Dodge perpendicular to projectile direction
                glm::vec2 projDir = Utils::Normalize(proj->velocity);
                dodgeDirection = glm::vec2(-projDir.y, projDir.x);
                if (Utils::RandomFloat(0.0f, 1.0f) < 0.5f) {
                    dodgeDirection = -dodgeDirection;
                }
                isDodging = true;
                dodgeTimer = 0.3f;
                dodgeCooldown = Utils::RandomFloat(1.5f, 3.0f);
                return;
            }
        }
        dodgeTimer = dodgeCooldown;
    }
    
    // Normal chase behavior
    glm::vec2 direction = Utils::Normalize(player->position - position);
    velocity = direction * moveSpeed;
    rotation = Utils::Angle(direction) + glm::half_pi<float>();
}

void FastEnemy::Render(Renderer* renderer) {
    // Sleek fast enemy
    renderer->DrawSpriteWorld(position, size, rotation, color, nullptr);
    
    // Trail effect when dodging
    if (isDodging) {
        glm::vec4 trailColor = color;
        trailColor.a = 0.3f;
        glm::vec2 trailPos = position - Utils::Normalize(velocity) * size.x;
        renderer->DrawSpriteWorld(trailPos, size * 0.8f, rotation, trailColor, nullptr);
    }
    
    RenderHealthBar(renderer);
}
