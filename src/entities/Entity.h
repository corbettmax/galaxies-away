#ifndef ENTITY_H
#define ENTITY_H

#include "../utils.h"
#include "../renderer.h"

// Forward declaration
class Game;

// ============================================================================
// Base Entity Class
// ============================================================================

class Entity {
public:
    glm::vec2 position;
    glm::vec2 velocity;
    glm::vec2 size;
    float rotation;
    float radius;           // Collision radius
    glm::vec4 color;
    EntityType type;
    bool active;
    bool markedForDeletion;
    
    Entity();
    virtual ~Entity() = default;
    
    virtual void Update(float deltaTime, Game* game);
    virtual void Render(Renderer* renderer);
    virtual void OnCollision(Entity* other, Game* game);
    
    bool CollidesWith(Entity* other) const;
    float DistanceTo(Entity* other) const;
    float DistanceTo(const glm::vec2& point) const;
};

#endif // ENTITY_H
