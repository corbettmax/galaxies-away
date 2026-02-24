#include "Player.h"
#include "../game.h"

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
