#ifndef XPORB_H
#define XPORB_H

// ============================================================================
// XP Orb Entity
// Collectible experience orbs dropped by defeated enemies
// ============================================================================

#include "Entity.h"

// Forward declarations
class Game;
class Renderer;

class XPOrb : public Entity {
public:
    int xpValue;
    float magnetSpeed;
    bool beingCollected;
    float lifetime;
    float pulseTimer;
    
    XPOrb(int value = 1);
    
    void Update(float deltaTime, Game* game) override;
    void Render(Renderer* renderer) override;
};

#endif // XPORB_H
