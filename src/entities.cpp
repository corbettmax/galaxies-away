// ============================================================================
// Galaxies Away - Entity System Implementation
// ============================================================================

#include "entities.h"
#include "game.h"

// ============================================================================
// Base Entity Implementation
// ============================================================================

Entity::Entity()
    : position(0.0f)
    , velocity(0.0f)
    , size(32.0f)
    , rotation(0.0f)
    , radius(16.0f)
    , color(Colors::WHITE)
    , type(EntityType::PLAYER)
    , active(true)
    , markedForDeletion(false)
{
}

void Entity::Update(float deltaTime, Game* game) {
    (void)game; // Suppress unused parameter warning
    position += velocity * deltaTime;
}

void Entity::Render(Renderer* renderer) {
    renderer->DrawSpriteWorld(position, size, rotation, color, nullptr);
}

void Entity::OnCollision(Entity* other, Game* game) {
    (void)other;
    (void)game;
}

bool Entity::CollidesWith(Entity* other) const {
    return Utils::CircleCollision(position, radius, other->position, other->radius);
}

float Entity::DistanceTo(Entity* other) const {
    return Utils::Distance(position, other->position);
}

float Entity::DistanceTo(const glm::vec2& point) const {
    return Utils::Distance(position, point);
}

// ============================================================================
// Player Implementation
// ============================================================================

Player::Player()
    : health(Constants::PLAYER_MAX_HEALTH)
    , maxHealth(Constants::PLAYER_MAX_HEALTH)
    , moveSpeed(Constants::PLAYER_SPEED)
    , pickupRadius(Constants::PLAYER_PICKUP_RADIUS)
    , experience(0)
    , level(1)
    , experienceToNextLevel(Constants::BASE_XP_REQUIREMENT)
    , damageMultiplier(1.0f)
    , fireRateMultiplier(1.0f)
    , projectileCountBonus(0)
    , projectileSizeMultiplier(1.0f)
    , moveInput(0.0f)
    , invincibilityTimer(0.0f)
    , invincibilityDuration(1.0f)
    , engineGlow(0.0f)
{
    type = EntityType::PLAYER;
    size = glm::vec2(Constants::PLAYER_SIZE);
    radius = Constants::PLAYER_SIZE * 0.4f;
    color = Colors::PLAYER_BLUE;
}

void Player::Update(float deltaTime, Game* game) {
    // Update invincibility
    if (invincibilityTimer > 0.0f) {
        invincibilityTimer -= deltaTime;
    }
    
    // Update movement
    UpdateMovement(deltaTime);
    
    // Update engine glow based on movement
    float targetGlow = Utils::Length(moveInput) > 0.1f ? 1.0f : 0.3f;
    engineGlow = Utils::Lerp(engineGlow, targetGlow, deltaTime * 5.0f);
    
    // Clamp to world bounds
    float halfWidth = Constants::WORLD_WIDTH * 0.5f;
    float halfHeight = Constants::WORLD_HEIGHT * 0.5f;
    position.x = Utils::Clamp(position.x, -halfWidth + radius, halfWidth - radius);
    position.y = Utils::Clamp(position.y, -halfHeight + radius, halfHeight - radius);
    
    Entity::Update(deltaTime, game);
}

void Player::UpdateMovement(float deltaTime) {
    // Normalize diagonal movement
    glm::vec2 normalizedInput = Utils::Length(moveInput) > 1.0f ? Utils::Normalize(moveInput) : moveInput;
    
    // Apply movement
    glm::vec2 targetVelocity = normalizedInput * moveSpeed;
    
    // Smooth acceleration
    float acceleration = 10.0f;
    velocity = Utils::Lerp(velocity, targetVelocity, deltaTime * acceleration);
    
    // Update rotation to face movement direction (if moving)
    if (Utils::Length(velocity) > 10.0f) {
        float targetRotation = Utils::Angle(velocity) + glm::half_pi<float>();
        
        // Smooth rotation
        float rotDiff = targetRotation - rotation;
        while (rotDiff > glm::pi<float>()) rotDiff -= glm::two_pi<float>();
        while (rotDiff < -glm::pi<float>()) rotDiff += glm::two_pi<float>();
        rotation += rotDiff * deltaTime * 8.0f;
    }
}

void Player::Render(Renderer* renderer) {
    // Render engine trail first (behind ship)
    RenderEngineTrail(renderer);
    
    // Render ship
    RenderShip(renderer);
}

void Player::RenderShip(Renderer* renderer) {
    // Flash when invincible
    if (IsInvincible()) {
        float flash = std::sin(invincibilityTimer * 20.0f) * 0.5f + 0.5f;
        if (flash < 0.3f) return; // Skip rendering some frames for flashing effect
    }
    
    // Main ship body (triangle-ish shape using rotated quad)
    glm::vec4 shipColor = color;
    renderer->DrawSpriteWorld(position, size, rotation, shipColor, nullptr);
    
    // Cockpit (smaller, brighter)
    glm::vec4 cockpitColor = glm::vec4(0.5f, 0.8f, 1.0f, 1.0f);
    glm::vec2 cockpitOffset = Utils::RotateVector(glm::vec2(0.0f, -size.y * 0.15f), rotation);
    renderer->DrawSpriteWorld(position + cockpitOffset, size * 0.3f, rotation, cockpitColor, nullptr);
    
    // Wings
    glm::vec4 wingColor = glm::vec4(0.2f, 0.4f, 0.7f, 1.0f);
    glm::vec2 leftWingOffset = Utils::RotateVector(glm::vec2(-size.x * 0.4f, size.y * 0.1f), rotation);
    glm::vec2 rightWingOffset = Utils::RotateVector(glm::vec2(size.x * 0.4f, size.y * 0.1f), rotation);
    renderer->DrawSpriteWorld(position + leftWingOffset, size * glm::vec2(0.3f, 0.5f), rotation, wingColor, nullptr);
    renderer->DrawSpriteWorld(position + rightWingOffset, size * glm::vec2(0.3f, 0.5f), rotation, wingColor, nullptr);
}

void Player::RenderEngineTrail(Renderer* renderer) {
    // Engine glow behind ship
    glm::vec2 engineOffset = Utils::RotateVector(glm::vec2(0.0f, size.y * 0.5f), rotation);
    glm::vec2 enginePos = position + engineOffset;
    
    // Main engine glow
    glm::vec4 glowColor = glm::vec4(0.3f, 0.5f, 1.0f, 0.6f * engineGlow);
    renderer->DrawSpriteWorld(enginePos, size * 0.4f * engineGlow, rotation, glowColor, nullptr);
    
    // Hot center
    glm::vec4 hotColor = glm::vec4(0.8f, 0.9f, 1.0f, 0.8f * engineGlow);
    renderer->DrawSpriteWorld(enginePos, size * 0.2f * engineGlow, rotation, hotColor, nullptr);
}

void Player::TakeDamage(float damage, Game* game) {
    if (IsInvincible()) return;
    
    health -= damage;
    invincibilityTimer = invincibilityDuration;
    
    // Screen shake
    if (g_Renderer) {
        g_Renderer->SetScreenShake(8.0f, 0.2f);
    }
    
    // Spawn damage particles
    if (game) {
        game->GetEntityManager()->particleSystem.SpawnHitSparks(
            position, glm::vec2(0.0f), Colors::RED, 10);
    }
    
    if (health <= 0.0f) {
        health = 0.0f;
        // Game over handled by Game class
    }
}

void Player::AddExperience(int amount, Game* game) {
    experience += amount;
    
    while (experience >= experienceToNextLevel) {
        experience -= experienceToNextLevel;
        level++;
        experienceToNextLevel = GetExperienceForLevel(level);
        
        // Notify game of level up
        if (game) {
            game->OnPlayerLevelUp();
        }
        
        // Spawn level up particles
        if (game) {
            game->GetEntityManager()->particleSystem.SpawnLevelUp(position);
        }
    }
}

void Player::ApplyUpgrade(const UpgradeChoice& upgrade) {
    switch (upgrade.type) {
        case UpgradeType::DAMAGE:
            damageMultiplier += upgrade.value;
            break;
        case UpgradeType::FIRE_RATE:
            fireRateMultiplier += upgrade.value;
            break;
        case UpgradeType::PROJECTILE_COUNT:
            projectileCountBonus += static_cast<int>(upgrade.value);
            break;
        case UpgradeType::PROJECTILE_SIZE:
            projectileSizeMultiplier += upgrade.value;
            break;
        case UpgradeType::MOVE_SPEED:
            moveSpeed += upgrade.value;
            break;
        case UpgradeType::MAX_HEALTH:
            maxHealth += upgrade.value;
            health = std::min(health + upgrade.value, maxHealth);
            break;
        case UpgradeType::PICKUP_RADIUS:
            pickupRadius += upgrade.value;
            break;
        case UpgradeType::NEW_WEAPON:
            // Handled by Game class
            break;
    }
}

void Player::Heal(float amount) {
    health = std::min(health + amount, maxHealth);
}

int Player::GetExperienceForLevel(int lvl) const {
    return static_cast<int>(Constants::BASE_XP_REQUIREMENT * std::pow(Constants::XP_SCALING, lvl - 1));
}

void Player::OnCollision(Entity* other, Game* game) {
    // Player collision handling - damage is handled in TakeDamage
    // This is called when player collides with enemies
    if (other->type == EntityType::ENEMY_BASIC || 
        other->type == EntityType::ENEMY_TANK ||
        other->type == EntityType::ENEMY_FAST ||
        other->type == EntityType::ENEMY_BOSS) {
        // Damage is handled elsewhere via TakeDamage
        (void)game; // Suppress unused warning
    }
}

// ============================================================================
// Enemy Base Implementation
// ============================================================================

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

// ============================================================================
// BasicEnemy Implementation
// ============================================================================

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
    
    // Render the enemy with texture if available, otherwise use solid color
    if (enemyTex && enemyTex->textureID != 0) {
        // Scale up 4x and use white color to preserve original texture colors
        renderer->DrawSpriteWorld(position, size * 4.0f, rotation, Colors::WHITE, enemyTex);
    } else {
        // Fallback to solid color rendering
        renderer->DrawSpriteWorld(position, size, rotation, color, nullptr);
        
        // Eye/core
        glm::vec4 coreColor = glm::vec4(1.0f, 0.8f, 0.3f, 1.0f);
        renderer->DrawSpriteWorld(position, size * 0.3f, rotation, coreColor, nullptr);
    }
    
    RenderHealthBar(renderer);
}

// ============================================================================
// TankEnemy Implementation
// ============================================================================

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

// ============================================================================
// FastEnemy Implementation
// ============================================================================

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

// ============================================================================
// BossEnemy Implementation
// ============================================================================

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

// ============================================================================
// Projectile Implementation
// ============================================================================

Projectile::Projectile()
    : damage(10.0f)
    , lifetime(0.0f)
    , maxLifetime(2.0f)
    , isPlayerProjectile(true)
    , piercing(false)
    , pierceCount(0)
    , maxPierceCount(0)
    , homing(false)
    , homingStrength(0.0f)
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

// ============================================================================
// XPOrb Implementation
// ============================================================================

XPOrb::XPOrb(int value)
    : xpValue(value)
    , magnetSpeed(0.0f)
    , beingCollected(false)
    , lifetime(0.0f)
    , pulseTimer(0.0f)
{
    type = EntityType::XP_ORB;
    size = glm::vec2(12.0f + value * 2.0f);
    radius = size.x * 0.5f;
    color = Colors::XP_GREEN;
}

void XPOrb::Update(float deltaTime, Game* game) {
    lifetime += deltaTime;
    pulseTimer += deltaTime * 3.0f;
    
    if (!game || !game->GetEntityManager()->player) return;
    
    Player* player = game->GetEntityManager()->player.get();
    float distToPlayer = DistanceTo(player);
    
    // Check if within pickup radius
    if (distToPlayer < player->pickupRadius) {
        beingCollected = true;
    }
    
    // Move towards player if being collected
    if (beingCollected) {
        magnetSpeed += deltaTime * 2000.0f;
        magnetSpeed = std::min(magnetSpeed, 800.0f);
        
        glm::vec2 direction = Utils::Normalize(player->position - position);
        velocity = direction * magnetSpeed;
    } else {
        // Slow down over time
        velocity *= 0.95f;
    }
    
    Entity::Update(deltaTime, game);
}

void XPOrb::Render(Renderer* renderer) {
    float pulse = 0.8f + 0.2f * std::sin(pulseTimer);
    glm::vec2 renderSize = size * pulse;
    
    // Outer glow
    glm::vec4 glowColor = color;
    glowColor.a = 0.3f;
    renderer->DrawSpriteWorld(position, renderSize * 1.5f, 0.0f, glowColor, nullptr);
    
    // Main orb
    renderer->DrawSpriteWorld(position, renderSize, 0.0f, color, nullptr);
    
    // Bright center
    glm::vec4 centerColor = Colors::WHITE;
    centerColor.a = 0.8f;
    renderer->DrawSpriteWorld(position, renderSize * 0.4f, 0.0f, centerColor, nullptr);
}

// ============================================================================
// Particle Implementation
// ============================================================================

Particle::Particle()
    : lifetime(0.0f)
    , maxLifetime(1.0f)
    , startColor(Colors::WHITE)
    , endColor(glm::vec4(1.0f, 1.0f, 1.0f, 0.0f))
    , startSize(8.0f)
    , endSize(0.0f)
    , drag(0.98f)
{
    type = EntityType::PARTICLE;
    active = false;
}

void Particle::Update(float deltaTime, Game* game) {
    if (!active) return;
    
    lifetime += deltaTime;
    if (lifetime >= maxLifetime) {
        active = false;
        return;
    }
    
    // Apply drag
    velocity *= std::pow(drag, deltaTime * 60.0f);
    
    // Update visual properties
    float t = lifetime / maxLifetime;
    color = Utils::Lerp(startColor, endColor, t);
    float currentSize = Utils::Lerp(startSize, endSize, t);
    size = glm::vec2(currentSize);
    
    Entity::Update(deltaTime, game);
}

void Particle::Render(Renderer* renderer) {
    if (!active) return;
    renderer->DrawParticle(position, size.x, color);
}

// ============================================================================
// ParticleSystem Implementation
// ============================================================================

ParticleSystem::ParticleSystem() {
    particles.reserve(Constants::MAX_PARTICLES);
    for (int i = 0; i < Constants::MAX_PARTICLES; ++i) {
        particles.push_back(std::make_unique<Particle>());
    }
}

void ParticleSystem::Update(float deltaTime, Game* game) {
    for (auto& particle : particles) {
        if (particle->active) {
            particle->Update(deltaTime, game);
        }
    }
}

void ParticleSystem::Render(Renderer* renderer) {
    for (auto& particle : particles) {
        if (particle->active) {
            particle->Render(renderer);
        }
    }
    renderer->FlushParticles();
}

Particle* ParticleSystem::GetAvailableParticle() {
    for (auto& particle : particles) {
        if (!particle->active) {
            return particle.get();
        }
    }
    return nullptr;
}

void ParticleSystem::SpawnExplosion(const glm::vec2& position, const glm::vec4& color, int count, float speed) {
    for (int i = 0; i < count; ++i) {
        Particle* p = GetAvailableParticle();
        if (!p) break;
        
        p->active = true;
        p->position = position;
        p->velocity = Utils::RandomDirection() * Utils::RandomFloat(speed * 0.3f, speed);
        p->lifetime = 0.0f;
        p->maxLifetime = Utils::RandomFloat(0.3f, 0.8f);
        p->startColor = color;
        p->endColor = glm::vec4(color.r, color.g, color.b, 0.0f);
        p->startSize = Utils::RandomFloat(4.0f, 12.0f);
        p->endSize = 0.0f;
        p->drag = 0.95f;
    }
}

void ParticleSystem::SpawnHitSparks(const glm::vec2& position, const glm::vec2& direction, const glm::vec4& color, int count) {
    for (int i = 0; i < count; ++i) {
        Particle* p = GetAvailableParticle();
        if (!p) break;
        
        p->active = true;
        p->position = position;
        
        glm::vec2 sparkDir = direction;
        if (Utils::Length(sparkDir) < 0.1f) {
            sparkDir = Utils::RandomDirection();
        }
        sparkDir = Utils::Normalize(sparkDir + Utils::RandomDirection() * 0.5f);
        
        p->velocity = sparkDir * Utils::RandomFloat(100.0f, 250.0f);
        p->lifetime = 0.0f;
        p->maxLifetime = Utils::RandomFloat(0.1f, 0.3f);
        p->startColor = color;
        p->endColor = glm::vec4(color.r, color.g, color.b, 0.0f);
        p->startSize = Utils::RandomFloat(2.0f, 6.0f);
        p->endSize = 0.0f;
        p->drag = 0.9f;
    }
}

void ParticleSystem::SpawnTrail(const glm::vec2& position, const glm::vec4& color, float particleSize) {
    Particle* p = GetAvailableParticle();
    if (!p) return;
    
    p->active = true;
    p->position = position + Utils::RandomPointInCircle(3.0f);
    p->velocity = Utils::RandomDirection() * Utils::RandomFloat(10.0f, 30.0f);
    p->lifetime = 0.0f;
    p->maxLifetime = Utils::RandomFloat(0.2f, 0.4f);
    p->startColor = color;
    p->endColor = glm::vec4(color.r, color.g, color.b, 0.0f);
    p->startSize = particleSize;
    p->endSize = 0.0f;
    p->drag = 0.98f;
}

void ParticleSystem::SpawnLevelUp(const glm::vec2& position) {
    // Ring of particles
    int count = 30;
    for (int i = 0; i < count; ++i) {
        Particle* p = GetAvailableParticle();
        if (!p) break;
        
        float angle = (glm::two_pi<float>() / count) * i;
        glm::vec2 dir = glm::vec2(std::cos(angle), std::sin(angle));
        
        p->active = true;
        p->position = position;
        p->velocity = dir * 300.0f;
        p->lifetime = 0.0f;
        p->maxLifetime = 0.5f;
        p->startColor = Colors::YELLOW;
        p->endColor = glm::vec4(1.0f, 1.0f, 0.2f, 0.0f);
        p->startSize = 8.0f;
        p->endSize = 2.0f;
        p->drag = 0.92f;
    }
}

void ParticleSystem::SpawnXPCollect(const glm::vec2& position) {
    for (int i = 0; i < 5; ++i) {
        Particle* p = GetAvailableParticle();
        if (!p) break;
        
        p->active = true;
        p->position = position;
        p->velocity = Utils::RandomDirection() * Utils::RandomFloat(50.0f, 100.0f);
        p->lifetime = 0.0f;
        p->maxLifetime = 0.3f;
        p->startColor = Colors::XP_GREEN;
        p->endColor = glm::vec4(0.4f, 1.0f, 0.4f, 0.0f);
        p->startSize = 4.0f;
        p->endSize = 0.0f;
        p->drag = 0.95f;
    }
}

void ParticleSystem::Clear() {
    for (auto& particle : particles) {
        particle->active = false;
    }
}

int ParticleSystem::GetActiveCount() const {
    int count = 0;
    for (const auto& particle : particles) {
        if (particle->active) count++;
    }
    return count;
}

// ============================================================================
// EntityManager Implementation
// ============================================================================

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
