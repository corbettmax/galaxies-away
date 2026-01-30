// ============================================================================
// Galaxies Away - Weapons System Implementation
// ============================================================================

#include "weapons.h"
#include "game.h"

// ============================================================================
// Base Weapon Implementation
// ============================================================================

Weapon::Weapon()
    : type(WeaponType::LASER)
    , name("Unknown")
    , level(1)
    , baseDamage(10.0f)
    , baseFireRate(2.0f)
    , baseProjectileSpeed(500.0f)
    , baseProjectileCount(1)
    , baseProjectileSize(8.0f)
    , damage(10.0f)
    , fireRate(2.0f)
    , projectileSpeed(500.0f)
    , projectileCount(1)
    , projectileSize(8.0f)
    , fireTimer(0.0f)
    , fireCooldown(0.5f)
    , projectileColor(Colors::CYAN)
{
}

void Weapon::Update(float deltaTime, Game* game) {
    // Update fire timer
    if (fireTimer > 0.0f) {
        fireTimer -= deltaTime;
    }
    
    // Auto-fire when ready
    if (CanFire() && game && game->GetEntityManager()->player) {
        // Check if there are enemies to shoot at
        Enemy* target = game->GetEntityManager()->FindNearestEnemy(
            game->GetEntityManager()->player->position, 600.0f);
        
        if (target) {
            Fire(game);
            ResetFireTimer();
        }
    }
}

void Weapon::UpdateStats(Player* player) {
    if (!player) return;
    
    damage = baseDamage * player->damageMultiplier;
    fireRate = baseFireRate * player->fireRateMultiplier;
    fireCooldown = 1.0f / fireRate;
    projectileCount = baseProjectileCount + player->projectileCountBonus;
    projectileSize = baseProjectileSize * player->projectileSizeMultiplier;
}

void Weapon::LevelUp() {
    level++;
    baseDamage *= 1.2f;
    baseFireRate *= 1.1f;
}

std::string Weapon::GetDescription() const {
    return name + " Lv." + std::to_string(level);
}

// ============================================================================
// Laser Weapon Implementation
// ============================================================================

LaserWeapon::LaserWeapon()
    : piercing(false)
    , pierceCount(1)
{
    type = WeaponType::LASER;
    name = "Laser";
    baseDamage = 12.0f;
    baseFireRate = 3.0f;
    baseProjectileSpeed = 600.0f;
    baseProjectileSize = 10.0f;
    projectileColor = glm::vec4(0.3f, 0.8f, 1.0f, 1.0f);
    
    UpdateStats(nullptr);
}

void LaserWeapon::Fire(Game* game) {
    if (!game || !game->GetEntityManager()->player) return;
    
    EntityManager* em = game->GetEntityManager();
    Player* player = em->player.get();
    
    // Find nearest enemy
    Enemy* target = em->FindNearestEnemy(player->position, 600.0f);
    if (!target) return;
    
    // Calculate direction to target
    glm::vec2 direction = Utils::Normalize(target->position - player->position);
    
    // Fire projectiles
    for (int i = 0; i < projectileCount; ++i) {
        // Add slight spread for multiple projectiles
        float spreadAngle = 0.0f;
        if (projectileCount > 1) {
            float totalSpread = 0.3f;  // ~17 degrees total
            spreadAngle = -totalSpread / 2.0f + (totalSpread / (projectileCount - 1)) * i;
        }
        
        glm::vec2 shotDir = Utils::RotateVector(direction, spreadAngle);
        glm::vec2 velocity = shotDir * projectileSpeed;
        
        Projectile* proj = em->SpawnProjectile(
            player->position + shotDir * player->radius,
            velocity,
            damage,
            true
        );
        
        proj->color = projectileColor;
        proj->size = glm::vec2(projectileSize, projectileSize * 0.5f);
        proj->maxLifetime = 2.0f;
        
        if (piercing) {
            proj->SetPiercing(pierceCount);
        }
    }
}

void LaserWeapon::LevelUp() {
    Weapon::LevelUp();
    
    // Every 3 levels, gain pierce
    if (level % 3 == 0) {
        if (!piercing) {
            piercing = true;
            pierceCount = 2;
        } else {
            pierceCount++;
        }
    }
    
    // Every 2 levels, increase projectile count
    if (level % 2 == 0) {
        baseProjectileCount++;
    }
}

std::string LaserWeapon::GetDescription() const {
    std::string desc = "Laser Lv." + std::to_string(level);
    if (piercing) {
        desc += " (Pierce x" + std::to_string(pierceCount) + ")";
    }
    return desc;
}

// ============================================================================
// Missile Weapon Implementation
// ============================================================================

MissileWeapon::MissileWeapon()
    : homingStrength(5.0f)
    , explosionRadius(50.0f)
{
    type = WeaponType::MISSILE;
    name = "Missile";
    baseDamage = 25.0f;
    baseFireRate = 1.0f;
    baseProjectileSpeed = 300.0f;
    baseProjectileSize = 14.0f;
    projectileColor = glm::vec4(1.0f, 0.6f, 0.2f, 1.0f);
    
    UpdateStats(nullptr);
}

void MissileWeapon::Fire(Game* game) {
    if (!game || !game->GetEntityManager()->player) return;
    
    EntityManager* em = game->GetEntityManager();
    Player* player = em->player.get();
    
    for (int i = 0; i < projectileCount; ++i) {
        // Fire in different directions
        float angle = (glm::two_pi<float>() / projectileCount) * i + player->rotation;
        glm::vec2 direction = glm::vec2(std::cos(angle), std::sin(angle));
        glm::vec2 velocity = direction * projectileSpeed;
        
        Projectile* proj = em->SpawnProjectile(
            player->position + direction * player->radius,
            velocity,
            damage,
            true
        );
        
        proj->color = projectileColor;
        proj->size = glm::vec2(projectileSize);
        proj->maxLifetime = 4.0f;
        proj->SetHoming(homingStrength);
    }
}

void MissileWeapon::LevelUp() {
    Weapon::LevelUp();
    
    homingStrength += 0.5f;
    explosionRadius += 10.0f;
    
    // Every 3 levels, add a missile
    if (level % 3 == 0) {
        baseProjectileCount++;
    }
}

std::string MissileWeapon::GetDescription() const {
    return "Missile Lv." + std::to_string(level) + " (Homing)";
}

// ============================================================================
// Orbital Weapon Implementation
// ============================================================================

OrbitalWeapon::OrbitalWeapon()
    : orbitSpeed(3.0f)
    , orbitRadius(80.0f)
    , satelliteSize(20.0f)
    , contactDamage(15.0f)
{
    type = WeaponType::ORBITAL;
    name = "Orbital";
    baseDamage = 15.0f;
    baseFireRate = 0.0f;  // Not projectile-based
    
    // Start with one satellite
    OrbitalSatellite sat;
    sat.angle = 0.0f;
    sat.orbitRadius = orbitRadius;
    satellites.push_back(sat);
}

void OrbitalWeapon::Update(float deltaTime, Game* game) {
    UpdateSatellites(deltaTime, game);
    CheckSatelliteCollisions(game);
}

void OrbitalWeapon::UpdateSatellites(float deltaTime, Game* game) {
    if (!game || !game->GetEntityManager()->player) return;
    
    Player* player = game->GetEntityManager()->player.get();
    
    for (auto& sat : satellites) {
        // Update angle
        sat.angle += orbitSpeed * deltaTime;
        if (sat.angle > glm::two_pi<float>()) {
            sat.angle -= glm::two_pi<float>();
        }
        
        // Update position
        sat.position = player->position + glm::vec2(
            std::cos(sat.angle) * sat.orbitRadius,
            std::sin(sat.angle) * sat.orbitRadius
        );
        
        // Update damage cooldown
        if (sat.damageTimer > 0.0f) {
            sat.damageTimer -= deltaTime;
        }
    }
}

void OrbitalWeapon::CheckSatelliteCollisions(Game* game) {
    if (!game) return;
    
    EntityManager* em = game->GetEntityManager();
    
    for (auto& sat : satellites) {
        if (sat.damageTimer > 0.0f) continue;
        
        // Check collision with enemies
        for (auto& enemy : em->enemies) {
            if (!enemy->active || enemy->markedForDeletion) continue;
            
            float dist = Utils::Distance(sat.position, enemy->position);
            if (dist < satelliteSize * 0.5f + enemy->radius) {
                enemy->TakeDamage(contactDamage * (game->GetEntityManager()->player ? 
                    game->GetEntityManager()->player->damageMultiplier : 1.0f), game);
                sat.damageTimer = sat.damageCooldown;
                
                // Spawn hit effect
                em->particleSystem.SpawnHitSparks(sat.position, glm::vec2(0.0f), Colors::CYAN, 5);
                break;
            }
        }
    }
}

void OrbitalWeapon::Fire(Game* game) {
    // Add a new satellite
    (void)game;
    OrbitalSatellite sat;
    sat.angle = satellites.empty() ? 0.0f : satellites.back().angle + glm::pi<float>() / 2.0f;
    sat.orbitRadius = orbitRadius;
    satellites.push_back(sat);
}

void OrbitalWeapon::LevelUp() {
    level++;
    contactDamage *= 1.2f;
    orbitSpeed += 0.3f;
    
    // Every 2 levels, add a satellite
    if (level % 2 == 0 && satellites.size() < 8) {
        OrbitalSatellite sat;
        sat.angle = satellites.back().angle + glm::two_pi<float>() / (satellites.size() + 1);
        sat.orbitRadius = orbitRadius;
        satellites.push_back(sat);
    }
    
    // Every 3 levels, increase orbit radius
    if (level % 3 == 0) {
        orbitRadius += 15.0f;
        for (auto& sat : satellites) {
            sat.orbitRadius = orbitRadius;
        }
    }
}

std::string OrbitalWeapon::GetDescription() const {
    return "Orbital Lv." + std::to_string(level) + " (" + std::to_string(satellites.size()) + " sats)";
}

void OrbitalWeapon::Render(Renderer* renderer, const glm::vec2& playerPos) {
    (void)playerPos;
    
    for (const auto& sat : satellites) {
        // Glow
        glm::vec4 glowColor = glm::vec4(0.3f, 0.7f, 1.0f, 0.3f);
        renderer->DrawSpriteWorld(sat.position, glm::vec2(satelliteSize * 1.5f), 0.0f, glowColor, nullptr);
        
        // Main body
        glm::vec4 bodyColor = glm::vec4(0.5f, 0.8f, 1.0f, 1.0f);
        renderer->DrawSpriteWorld(sat.position, glm::vec2(satelliteSize), sat.angle * 2.0f, bodyColor, nullptr);
        
        // Core
        glm::vec4 coreColor = Colors::WHITE;
        renderer->DrawSpriteWorld(sat.position, glm::vec2(satelliteSize * 0.4f), -sat.angle * 3.0f, coreColor, nullptr);
    }
}

// ============================================================================
// Shield Weapon Implementation
// ============================================================================

ShieldWeapon::ShieldWeapon()
    : shieldRadius(60.0f)
    , shieldDamage(5.0f)
    , damageTickRate(0.2f)
    , damageTimer(0.0f)
    , shieldHealth(50.0f)
    , maxShieldHealth(50.0f)
    , regenRate(10.0f)
    , regenDelay(2.0f)
    , regenTimer(0.0f)
    , shieldActive(true)
{
    type = WeaponType::SHIELD;
    name = "Shield";
    baseDamage = 5.0f;
    baseFireRate = 0.0f;
}

void ShieldWeapon::Update(float deltaTime, Game* game) {
    // Regenerate shield
    if (shieldHealth < maxShieldHealth) {
        regenTimer += deltaTime;
        if (regenTimer >= regenDelay) {
            shieldHealth += regenRate * deltaTime;
            shieldHealth = std::min(shieldHealth, maxShieldHealth);
        }
    }
    
    shieldActive = shieldHealth > 0.0f;
    
    // Damage enemies in shield
    if (shieldActive) {
        damageTimer -= deltaTime;
        if (damageTimer <= 0.0f) {
            DamageEnemiesInShield(game);
            damageTimer = damageTickRate;
        }
    }
}

void ShieldWeapon::DamageEnemiesInShield(Game* game) {
    if (!game || !game->GetEntityManager()->player) return;
    
    EntityManager* em = game->GetEntityManager();
    Player* player = em->player.get();
    
    float actualDamage = shieldDamage * player->damageMultiplier;
    
    auto enemies = em->FindEnemiesInRange(player->position, shieldRadius);
    for (Enemy* enemy : enemies) {
        enemy->TakeDamage(actualDamage, game);
    }
}

void ShieldWeapon::Fire(Game* game) {
    // Shield pulse - knockback nearby enemies
    if (!game || !game->GetEntityManager()->player) return;
    
    EntityManager* em = game->GetEntityManager();
    Player* player = em->player.get();
    
    auto enemies = em->FindEnemiesInRange(player->position, shieldRadius * 1.5f);
    for (Enemy* enemy : enemies) {
        glm::vec2 knockback = Utils::Normalize(enemy->position - player->position) * 200.0f;
        enemy->velocity += knockback;
        enemy->TakeDamage(shieldDamage * 3.0f * player->damageMultiplier, game);
    }
    
    // Spawn pulse effect
    em->particleSystem.SpawnExplosion(player->position, Colors::SHIELD_CYAN, 30, 200.0f);
}

void ShieldWeapon::LevelUp() {
    level++;
    shieldRadius += 10.0f;
    shieldDamage *= 1.2f;
    maxShieldHealth += 20.0f;
    shieldHealth = maxShieldHealth;
    regenRate += 2.0f;
}

std::string ShieldWeapon::GetDescription() const {
    return "Shield Lv." + std::to_string(level);
}

void ShieldWeapon::Render(Renderer* renderer, const glm::vec2& playerPos) {
    if (!shieldActive) return;
    
    float healthPercent = shieldHealth / maxShieldHealth;
    float alpha = 0.2f + 0.3f * healthPercent;
    
    // Outer shield
    glm::vec4 shieldColor = Colors::SHIELD_CYAN;
    shieldColor.a = alpha;
    renderer->DrawSpriteWorld(playerPos, glm::vec2(shieldRadius * 2.0f), 0.0f, shieldColor, nullptr);
    
    // Inner glow
    glm::vec4 innerColor = Colors::SHIELD_CYAN;
    innerColor.a = alpha * 0.5f;
    renderer->DrawSpriteWorld(playerPos, glm::vec2(shieldRadius * 1.5f), 0.0f, innerColor, nullptr);
}

void ShieldWeapon::TakeShieldDamage(float damage) {
    shieldHealth -= damage;
    regenTimer = 0.0f;
    
    if (shieldHealth < 0.0f) {
        shieldHealth = 0.0f;
    }
}

// ============================================================================
// Plasma Weapon Implementation
// ============================================================================

PlasmaWeapon::PlasmaWeapon()
    : explosionRadius(80.0f)
    , chargeTime(0.5f)
    , chargeTimer(0.0f)
    , isCharging(false)
{
    type = WeaponType::PLASMA;
    name = "Plasma";
    baseDamage = 30.0f;
    baseFireRate = 0.8f;
    projectileColor = glm::vec4(0.8f, 0.3f, 1.0f, 1.0f);
    
    UpdateStats(nullptr);
}

void PlasmaWeapon::Update(float deltaTime, Game* game) {
    if (fireTimer > 0.0f) {
        fireTimer -= deltaTime;
    }
    
    // Auto-fire targeting nearest enemy cluster
    if (CanFire() && game && game->GetEntityManager()->player) {
        Enemy* target = game->GetEntityManager()->FindNearestEnemy(
            game->GetEntityManager()->player->position, 400.0f);
        
        if (target) {
            CreateExplosion(game, target->position);
            ResetFireTimer();
        }
    }
}

void PlasmaWeapon::Fire(Game* game) {
    if (!game || !game->GetEntityManager()->player) return;
    
    Enemy* target = game->GetEntityManager()->FindNearestEnemy(
        game->GetEntityManager()->player->position, 400.0f);
    
    if (target) {
        CreateExplosion(game, target->position);
    }
}

void PlasmaWeapon::CreateExplosion(Game* game, const glm::vec2& position) {
    if (!game) return;
    
    EntityManager* em = game->GetEntityManager();
    Player* player = em->player.get();
    
    float actualDamage = damage * (player ? player->damageMultiplier : 1.0f);
    
    // Damage all enemies in radius
    auto enemies = em->FindEnemiesInRange(position, explosionRadius);
    for (Enemy* enemy : enemies) {
        // Damage falloff based on distance
        float dist = Utils::Distance(position, enemy->position);
        float falloff = 1.0f - (dist / explosionRadius) * 0.5f;
        enemy->TakeDamage(actualDamage * falloff, game);
    }
    
    // Spawn explosion particles
    em->particleSystem.SpawnExplosion(position, projectileColor, 40, 250.0f);
    
    // Screen shake
    if (g_Renderer) {
        g_Renderer->SetScreenShake(5.0f, 0.15f);
    }
}

void PlasmaWeapon::LevelUp() {
    Weapon::LevelUp();
    explosionRadius += 15.0f;
    
    // Every 2 levels, faster fire rate
    if (level % 2 == 0) {
        baseFireRate += 0.2f;
    }
}

std::string PlasmaWeapon::GetDescription() const {
    return "Plasma Lv." + std::to_string(level) + " (AOE)";
}

// ============================================================================
// Spread Weapon Implementation
// ============================================================================

SpreadWeapon::SpreadWeapon()
    : spreadAngle(glm::radians(60.0f))
    , pelletCount(5)
{
    type = WeaponType::SPREAD;
    name = "Spread";
    baseDamage = 8.0f;
    baseFireRate = 1.5f;
    baseProjectileSpeed = 450.0f;
    baseProjectileSize = 6.0f;
    projectileColor = glm::vec4(1.0f, 0.9f, 0.3f, 1.0f);
    
    UpdateStats(nullptr);
}

void SpreadWeapon::Fire(Game* game) {
    if (!game || !game->GetEntityManager()->player) return;
    
    EntityManager* em = game->GetEntityManager();
    Player* player = em->player.get();
    
    // Find nearest enemy for base direction
    Enemy* target = em->FindNearestEnemy(player->position, 500.0f);
    
    glm::vec2 baseDirection;
    if (target) {
        baseDirection = Utils::Normalize(target->position - player->position);
    } else {
        // Fire in facing direction
        baseDirection = glm::vec2(std::cos(player->rotation - glm::half_pi<float>()), 
                                  std::sin(player->rotation - glm::half_pi<float>()));
    }
    
    int totalPellets = pelletCount + projectileCount - 1;
    
    for (int i = 0; i < totalPellets; ++i) {
        // Calculate spread angle for this pellet
        float angleOffset = 0.0f;
        if (totalPellets > 1) {
            angleOffset = -spreadAngle / 2.0f + (spreadAngle / (totalPellets - 1)) * i;
        }
        
        glm::vec2 pelletDir = Utils::RotateVector(baseDirection, angleOffset);
        
        // Add small random variation
        float randomOffset = Utils::RandomFloat(-0.05f, 0.05f);
        pelletDir = Utils::RotateVector(pelletDir, randomOffset);
        
        glm::vec2 velocity = pelletDir * projectileSpeed;
        
        Projectile* proj = em->SpawnProjectile(
            player->position + pelletDir * player->radius,
            velocity,
            damage,
            true
        );
        
        proj->color = projectileColor;
        proj->size = glm::vec2(projectileSize);
        proj->maxLifetime = 1.0f;  // Shorter range
    }
}

void SpreadWeapon::LevelUp() {
    Weapon::LevelUp();
    
    // Every 2 levels, add pellets
    if (level % 2 == 0) {
        pelletCount += 2;
    }
    
    // Every 3 levels, wider spread
    if (level % 3 == 0) {
        spreadAngle += glm::radians(10.0f);
    }
}

std::string SpreadWeapon::GetDescription() const {
    return "Spread Lv." + std::to_string(level) + " (" + std::to_string(pelletCount) + " pellets)";
}

// ============================================================================
// Weapon Manager Implementation
// ============================================================================

WeaponManager::WeaponManager() {
    // Start with basic laser weapon
    AddWeapon(WeaponType::LASER);
}

void WeaponManager::Update(float deltaTime, Game* game) {
    for (auto& weapon : weapons) {
        weapon->Update(deltaTime, game);
    }
}

void WeaponManager::Render(Renderer* renderer, const glm::vec2& playerPos) {
    // Render orbital weapons
    for (auto& weapon : weapons) {
        if (weapon->type == WeaponType::ORBITAL) {
            static_cast<OrbitalWeapon*>(weapon.get())->Render(renderer, playerPos);
        } else if (weapon->type == WeaponType::SHIELD) {
            static_cast<ShieldWeapon*>(weapon.get())->Render(renderer, playerPos);
        }
    }
}

bool WeaponManager::AddWeapon(WeaponType type) {
    if (HasWeapon(type)) {
        UpgradeWeapon(type);
        return false;
    }
    
    auto weapon = CreateWeapon(type);
    if (weapon) {
        weapons.push_back(std::move(weapon));
        return true;
    }
    return false;
}

bool WeaponManager::HasWeapon(WeaponType type) const {
    for (const auto& weapon : weapons) {
        if (weapon->type == type) return true;
    }
    return false;
}

Weapon* WeaponManager::GetWeapon(WeaponType type) {
    for (auto& weapon : weapons) {
        if (weapon->type == type) return weapon.get();
    }
    return nullptr;
}

void WeaponManager::UpgradeWeapon(WeaponType type) {
    Weapon* weapon = GetWeapon(type);
    if (weapon) {
        weapon->LevelUp();
    }
}

void WeaponManager::UpdateAllStats(Player* player) {
    for (auto& weapon : weapons) {
        weapon->UpdateStats(player);
    }
}

std::vector<UpgradeChoice> WeaponManager::GetAvailableUpgrades() const {
    std::vector<UpgradeChoice> choices;
    
    // Stat upgrades (always available)
    choices.push_back(UpgradeChoice("+20% Damage", "Increase all weapon damage", UpgradeType::DAMAGE, 0.2f));
    choices.push_back(UpgradeChoice("+15% Fire Rate", "Shoot faster", UpgradeType::FIRE_RATE, 0.15f));
    choices.push_back(UpgradeChoice("+1 Projectile", "Fire additional projectiles", UpgradeType::PROJECTILE_COUNT, 1.0f));
    choices.push_back(UpgradeChoice("+20% Proj Size", "Bigger projectiles", UpgradeType::PROJECTILE_SIZE, 0.2f));
    choices.push_back(UpgradeChoice("+10% Move Speed", "Move faster", UpgradeType::MOVE_SPEED, 25.0f));
    choices.push_back(UpgradeChoice("+25 Max Health", "Increase maximum health", UpgradeType::MAX_HEALTH, 25.0f));
    choices.push_back(UpgradeChoice("+30% Pickup Range", "Collect XP from further", UpgradeType::PICKUP_RADIUS, 24.0f));
    
    // New weapons (if not already owned)
    if (!HasWeapon(WeaponType::MISSILE)) {
        choices.push_back(UpgradeChoice("Homing Missiles", "Missiles that seek enemies", WeaponType::MISSILE));
    }
    if (!HasWeapon(WeaponType::ORBITAL)) {
        choices.push_back(UpgradeChoice("Orbital Drones", "Satellites that orbit you", WeaponType::ORBITAL));
    }
    if (!HasWeapon(WeaponType::SHIELD)) {
        choices.push_back(UpgradeChoice("Energy Shield", "Protective damage aura", WeaponType::SHIELD));
    }
    if (!HasWeapon(WeaponType::PLASMA)) {
        choices.push_back(UpgradeChoice("Plasma Bombs", "AOE explosions", WeaponType::PLASMA));
    }
    if (!HasWeapon(WeaponType::SPREAD)) {
        choices.push_back(UpgradeChoice("Spread Shot", "Shotgun-style spread", WeaponType::SPREAD));
    }
    
    // Weapon level-ups (if owned)
    for (const auto& weapon : weapons) {
        if (weapon->level < 8) {  // Max level cap
            std::string name = weapon->name + " Upgrade";
            std::string desc = "Level up to Lv." + std::to_string(weapon->level + 1);
            
            UpgradeChoice choice;
            choice.name = name;
            choice.description = desc;
            choice.type = UpgradeType::NEW_WEAPON;
            choice.weaponType = weapon->type;
            choice.value = -1.0f;  // Indicates upgrade, not new weapon
            choices.push_back(choice);
        }
    }
    
    return choices;
}

std::unique_ptr<Weapon> WeaponManager::CreateWeapon(WeaponType type) {
    switch (type) {
        case WeaponType::LASER:
            return std::make_unique<LaserWeapon>();
        case WeaponType::MISSILE:
            return std::make_unique<MissileWeapon>();
        case WeaponType::ORBITAL:
            return std::make_unique<OrbitalWeapon>();
        case WeaponType::SHIELD:
            return std::make_unique<ShieldWeapon>();
        case WeaponType::PLASMA:
            return std::make_unique<PlasmaWeapon>();
        case WeaponType::SPREAD:
            return std::make_unique<SpreadWeapon>();
        default:
            return nullptr;
    }
}
