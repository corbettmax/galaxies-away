// ============================================================================
// EntityManager Implementation
// ============================================================================

#include "EntityManager.h"
#include "BasicEnemy.h"
#include "TankEnemy.h"
#include "FastEnemy.h"
#include "BossEnemy.h"
#include "../game.h"
#include "../utils.h"
#include <algorithm>
#include <limits>

EntityManager::EntityManager() {
}

void EntityManager::Update(float deltaTime, Game* game) {
    // Update player
    if (player) {
        player->Update(deltaTime, game);
    }
    
    // Update enemies
    for (auto& enemy : enemies) {
        if (enemy->active) {
            enemy->Update(deltaTime, game);
        }
    }
    
    // Update projectiles
    for (auto& projectile : projectiles) {
        if (projectile->active) {
            projectile->Update(deltaTime, game);
        }
    }
    
    // Update XP orbs
    for (auto& orb : xpOrbs) {
        if (orb->active) {
            orb->Update(deltaTime, game);
        }
    }
    
    // Update particles
    particleSystem.Update(deltaTime, game);
    
    // Check collisions
    UpdateCollisions(game);
    
    // Cleanup dead entities
    CleanupDeadEntities();
}

void EntityManager::Render(Renderer* renderer) {
    // Render XP orbs (behind everything else)
    for (auto& orb : xpOrbs) {
        if (orb->active) {
            orb->Render(renderer);
        }
    }
    
    // Render enemies
    for (auto& enemy : enemies) {
        if (enemy->active) {
            enemy->Render(renderer);
        }
    }
    
    // Render player
    if (player) {
        player->Render(renderer);
    }
    
    // Render projectiles
    for (auto& projectile : projectiles) {
        if (projectile->active) {
            projectile->Render(renderer);
        }
    }
    
    // Render particles (on top)
    particleSystem.Render(renderer);
}

void EntityManager::SpawnPlayer(const glm::vec2& position) {
    player = std::make_unique<Player>();
    player->position = position;
}

Enemy* EntityManager::SpawnEnemy(EntityType enemyType, const glm::vec2& position) {
    std::unique_ptr<Enemy> enemy;
    
    switch (enemyType) {
        case EntityType::ENEMY_BASIC:
            enemy = std::make_unique<BasicEnemy>();
            break;
        case EntityType::ENEMY_TANK:
            enemy = std::make_unique<TankEnemy>();
            break;
        case EntityType::ENEMY_FAST:
            enemy = std::make_unique<FastEnemy>();
            break;
        case EntityType::ENEMY_BOSS:
            enemy = std::make_unique<BossEnemy>();
            break;
        default:
            enemy = std::make_unique<BasicEnemy>();
            break;
    }
    
    enemy->position = position;
    Enemy* ptr = enemy.get();
    enemies.push_back(std::move(enemy));
    return ptr;
}

Projectile* EntityManager::SpawnProjectile(const glm::vec2& position, const glm::vec2& velocity,
                                           float damage, bool isPlayerProjectile) {
    auto projectile = std::make_unique<Projectile>();
    projectile->position = position;
    projectile->velocity = velocity;
    projectile->damage = damage;
    projectile->isPlayerProjectile = isPlayerProjectile;
    projectile->color = isPlayerProjectile ? Colors::CYAN : Colors::ORANGE;
    
    Projectile* ptr = projectile.get();
    projectiles.push_back(std::move(projectile));
    return ptr;
}

void EntityManager::SpawnXPOrb(const glm::vec2& position, int value) {
    auto orb = std::make_unique<XPOrb>(value);
    orb->position = position;
    // Small random velocity
    orb->velocity = Utils::RandomDirection() * Utils::RandomFloat(30.0f, 80.0f);
    xpOrbs.push_back(std::move(orb));
}

Enemy* EntityManager::FindNearestEnemy(const glm::vec2& position, float maxRange) {
    Enemy* nearest = nullptr;
    float nearestDist = maxRange > 0 ? maxRange : std::numeric_limits<float>::max();
    
    for (auto& enemy : enemies) {
        if (!enemy->active || enemy->markedForDeletion) continue;
        
        float dist = Utils::Distance(position, enemy->position);
        if (dist < nearestDist) {
            nearestDist = dist;
            nearest = enemy.get();
        }
    }
    
    return nearest;
}

std::vector<Enemy*> EntityManager::FindEnemiesInRange(const glm::vec2& position, float range) {
    std::vector<Enemy*> result;
    
    for (auto& enemy : enemies) {
        if (!enemy->active || enemy->markedForDeletion) continue;
        
        if (Utils::Distance(position, enemy->position) <= range) {
            result.push_back(enemy.get());
        }
    }
    
    return result;
}

int EntityManager::GetEnemyCount() const {
    int count = 0;
    for (const auto& enemy : enemies) {
        if (enemy->active && !enemy->markedForDeletion) count++;
    }
    return count;
}

int EntityManager::GetProjectileCount() const {
    int count = 0;
    for (const auto& proj : projectiles) {
        if (proj->active && !proj->markedForDeletion) count++;
    }
    return count;
}

void EntityManager::UpdateCollisions(Game* game) {
    CheckProjectileEnemyCollisions(game);
    CheckPlayerEnemyCollisions(game);
    CheckPlayerXPCollisions(game);
}

void EntityManager::CheckProjectileEnemyCollisions(Game* game) {
    for (auto& projectile : projectiles) {
        if (!projectile->active || projectile->markedForDeletion) continue;
        
        if (projectile->isPlayerProjectile) {
            // Check against enemies
            for (auto& enemy : enemies) {
                if (!enemy->active || enemy->markedForDeletion) continue;
                
                if (projectile->CollidesWith(enemy.get())) {
                    projectile->OnCollision(enemy.get(), game);
                    if (projectile->markedForDeletion) break;
                }
            }
        } else {
            // Enemy projectile - check against player
            if (player && projectile->CollidesWith(player.get())) {
                projectile->OnCollision(player.get(), game);
            }
        }
    }
}

void EntityManager::CheckPlayerEnemyCollisions(Game* game) {
    if (!player || !player->active) return;
    
    for (auto& enemy : enemies) {
        if (!enemy->active || enemy->markedForDeletion) continue;
        
        if (player->CollidesWith(enemy.get())) {
            enemy->OnCollision(player.get(), game);
        }
    }
}

void EntityManager::CheckPlayerXPCollisions(Game* game) {
    if (!player || !player->active) return;
    
    for (auto& orb : xpOrbs) {
        if (!orb->active || orb->markedForDeletion) continue;
        
        if (player->CollidesWith(orb.get())) {
            player->AddExperience(orb->xpValue, game);
            particleSystem.SpawnXPCollect(orb->position);
            orb->markedForDeletion = true;
        }
    }
}

void EntityManager::CleanupDeadEntities() {
    // Remove dead enemies
    enemies.erase(
        std::remove_if(enemies.begin(), enemies.end(),
            [](const std::unique_ptr<Enemy>& e) { return e->markedForDeletion; }),
        enemies.end()
    );
    
    // Remove dead projectiles
    projectiles.erase(
        std::remove_if(projectiles.begin(), projectiles.end(),
            [](const std::unique_ptr<Projectile>& p) { return p->markedForDeletion; }),
        projectiles.end()
    );
    
    // Remove collected XP orbs
    xpOrbs.erase(
        std::remove_if(xpOrbs.begin(), xpOrbs.end(),
            [](const std::unique_ptr<XPOrb>& o) { return o->markedForDeletion; }),
        xpOrbs.end()
    );
}

void EntityManager::Clear() {
    enemies.clear();
    projectiles.clear();
    xpOrbs.clear();
    particleSystem.Clear();
}
