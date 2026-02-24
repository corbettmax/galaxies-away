#include "Entity.h"
#include "../game.h"

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
