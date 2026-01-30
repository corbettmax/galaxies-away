#ifndef WEAPONS_H
#define WEAPONS_H

// ============================================================================
// Galaxies Away - Weapons System
// Defines all weapons and their behaviors
// ============================================================================

#include "utils.h"
#include "entities.h"

// Forward declarations
class Game;
class EntityManager;

// ============================================================================
// Base Weapon Class
// ============================================================================

class Weapon {
public:
    WeaponType type;
    std::string name;
    int level;
    
    // Base stats
    float baseDamage;
    float baseFireRate;     // Shots per second
    float baseProjectileSpeed;
    int baseProjectileCount;
    float baseProjectileSize;
    
    // Current stats (affected by player upgrades)
    float damage;
    float fireRate;
    float projectileSpeed;
    int projectileCount;
    float projectileSize;
    
    // Timers
    float fireTimer;
    float fireCooldown;
    
    // Visual
    glm::vec4 projectileColor;
    
    Weapon();
    virtual ~Weapon() = default;
    
    virtual void Update(float deltaTime, Game* game);
    virtual void Fire(Game* game) = 0;
    virtual void UpdateStats(Player* player);
    
    bool CanFire() const { return fireTimer <= 0.0f; }
    void ResetFireTimer() { fireTimer = fireCooldown; }
    
    // Upgrade
    virtual void LevelUp();
    virtual std::string GetDescription() const;
};

// ============================================================================
// Laser Weapon
// Basic auto-targeting laser that can pierce enemies
// ============================================================================

class LaserWeapon : public Weapon {
public:
    bool piercing;
    int pierceCount;
    
    LaserWeapon();
    
    void Fire(Game* game) override;
    void LevelUp() override;
    std::string GetDescription() const override;
};

// ============================================================================
// Missile Weapon
// Homing missiles that seek out enemies
// ============================================================================

class MissileWeapon : public Weapon {
public:
    float homingStrength;
    float explosionRadius;
    
    MissileWeapon();
    
    void Fire(Game* game) override;
    void LevelUp() override;
    std::string GetDescription() const override;
};

// ============================================================================
// Orbital Weapon
// Satellites that orbit the player and damage enemies on contact
// ============================================================================

struct OrbitalSatellite {
    float angle;
    float orbitRadius;
    float damageTimer;
    float damageCooldown;
    glm::vec2 position;
    
    OrbitalSatellite() : angle(0.0f), orbitRadius(80.0f), damageTimer(0.0f), damageCooldown(0.3f) {}
};

class OrbitalWeapon : public Weapon {
public:
    std::vector<OrbitalSatellite> satellites;
    float orbitSpeed;
    float orbitRadius;
    float satelliteSize;
    float contactDamage;
    
    OrbitalWeapon();
    
    void Update(float deltaTime, Game* game) override;
    void Fire(Game* game) override;  // Adds a new satellite
    void LevelUp() override;
    std::string GetDescription() const override;
    
    void Render(Renderer* renderer, const glm::vec2& playerPos);
    
private:
    void UpdateSatellites(float deltaTime, Game* game);
    void CheckSatelliteCollisions(Game* game);
};

// ============================================================================
// Shield Weapon
// Protective shield that damages enemies on contact
// ============================================================================

class ShieldWeapon : public Weapon {
public:
    float shieldRadius;
    float shieldDamage;
    float damageTickRate;
    float damageTimer;
    float shieldHealth;
    float maxShieldHealth;
    float regenRate;
    float regenDelay;
    float regenTimer;
    bool shieldActive;
    
    ShieldWeapon();
    
    void Update(float deltaTime, Game* game) override;
    void Fire(Game* game) override;  // Activates shield pulse
    void LevelUp() override;
    std::string GetDescription() const override;
    
    void Render(Renderer* renderer, const glm::vec2& playerPos);
    void TakeShieldDamage(float damage);
    
private:
    void DamageEnemiesInShield(Game* game);
};

// ============================================================================
// Plasma Weapon
// AOE explosions that damage all enemies in radius
// ============================================================================

class PlasmaWeapon : public Weapon {
public:
    float explosionRadius;
    float chargeTime;
    float chargeTimer;
    bool isCharging;
    
    PlasmaWeapon();
    
    void Update(float deltaTime, Game* game) override;
    void Fire(Game* game) override;
    void LevelUp() override;
    std::string GetDescription() const override;
    
private:
    void CreateExplosion(Game* game, const glm::vec2& position);
};

// ============================================================================
// Spread Weapon
// Shotgun-style spread of projectiles
// ============================================================================

class SpreadWeapon : public Weapon {
public:
    float spreadAngle;  // Total spread angle in radians
    int pelletCount;
    
    SpreadWeapon();
    
    void Fire(Game* game) override;
    void LevelUp() override;
    std::string GetDescription() const override;
};

// ============================================================================
// Weapon Manager
// Handles all player weapons
// ============================================================================

class WeaponManager {
public:
    std::vector<std::unique_ptr<Weapon>> weapons;
    
    WeaponManager();
    
    void Update(float deltaTime, Game* game);
    void Render(Renderer* renderer, const glm::vec2& playerPos);
    
    // Weapon management
    bool AddWeapon(WeaponType type);
    bool HasWeapon(WeaponType type) const;
    Weapon* GetWeapon(WeaponType type);
    void UpgradeWeapon(WeaponType type);
    void UpdateAllStats(Player* player);
    
    // Get available upgrades for level-up menu
    std::vector<UpgradeChoice> GetAvailableUpgrades() const;
    
    int GetWeaponCount() const { return static_cast<int>(weapons.size()); }
    
private:
    std::unique_ptr<Weapon> CreateWeapon(WeaponType type);
};

#endif // WEAPONS_H
