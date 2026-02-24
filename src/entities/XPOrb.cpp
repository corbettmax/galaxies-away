// ============================================================================
// XPOrb Implementation
// ============================================================================

#include "XPOrb.h"
#include "Player.h"
#include "../game.h"
#include "../utils.h"
#include <algorithm>
#include <cmath>

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
