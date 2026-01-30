#ifndef UTILS_H
#define UTILS_H

// ============================================================================
// Galaxies Away - Utility Functions and Common Includes
// ============================================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <cmath>
#include <random>
#include <chrono>
#include <unordered_map>
#include <functional>

// GLM for mathematics
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Forward declarations
class Game;
class Renderer;
class Entity;
class Player;
class Enemy;
class Projectile;
class Particle;
class Weapon;

// ============================================================================
// Constants
// ============================================================================

namespace Constants {
    // Window settings
    constexpr int WINDOW_WIDTH = 1280;
    constexpr int WINDOW_HEIGHT = 720;
    constexpr const char* WINDOW_TITLE = "Galaxies Away";
    
    // Game world settings
    constexpr float WORLD_WIDTH = 3000.0f;
    constexpr float WORLD_HEIGHT = 3000.0f;
    
    // Player settings
    constexpr float PLAYER_SPEED = 250.0f;
    constexpr float PLAYER_MAX_HEALTH = 100.0f;
    constexpr float PLAYER_SIZE = 32.0f;
    constexpr float PLAYER_PICKUP_RADIUS = 80.0f;
    
    // XP and leveling
    constexpr int BASE_XP_REQUIREMENT = 10;
    constexpr float XP_SCALING = 1.5f;
    
    // Enemy spawning
    constexpr float BASE_SPAWN_RATE = 2.0f;  // seconds between spawns
    constexpr float MIN_SPAWN_RATE = 0.3f;
    constexpr float SPAWN_DISTANCE_MIN = 400.0f;
    constexpr float SPAWN_DISTANCE_MAX = 600.0f;
    
    // Visual settings
    constexpr int MAX_PARTICLES = 5000;
    constexpr float PARTICLE_LIFETIME = 1.0f;
}

// ============================================================================
// Game States
// ============================================================================

enum class GameState {
    MENU,
    PLAYING,
    LEVEL_UP,
    PAUSED,
    GAME_OVER
};

// ============================================================================
// Entity Types
// ============================================================================

enum class EntityType {
    PLAYER,
    ENEMY_BASIC,
    ENEMY_TANK,
    ENEMY_FAST,
    ENEMY_BOSS,
    PROJECTILE,
    XP_ORB,
    PARTICLE
};

// ============================================================================
// Weapon Types
// ============================================================================

enum class WeaponType {
    LASER,          // Basic laser, can pierce
    MISSILE,        // Homing missiles
    ORBITAL,        // Orbital satellites
    SHIELD,         // Damage shield
    PLASMA,         // AOE explosions
    SPREAD          // Shotgun-style spread
};

// ============================================================================
// Upgrade Types
// ============================================================================

enum class UpgradeType {
    DAMAGE,
    FIRE_RATE,
    PROJECTILE_COUNT,
    PROJECTILE_SIZE,
    MOVE_SPEED,
    MAX_HEALTH,
    PICKUP_RADIUS,
    NEW_WEAPON
};

// ============================================================================
// Upgrade/Weapon Choice for Level Up
// ============================================================================

struct UpgradeChoice {
    std::string name;
    std::string description;
    UpgradeType type;
    WeaponType weaponType;  // Only used if type == NEW_WEAPON
    float value;            // Multiplier or flat bonus
    
    UpgradeChoice() : type(UpgradeType::DAMAGE), weaponType(WeaponType::LASER), value(0) {}
    UpgradeChoice(const std::string& n, const std::string& d, UpgradeType t, float v)
        : name(n), description(d), type(t), weaponType(WeaponType::LASER), value(v) {}
    UpgradeChoice(const std::string& n, const std::string& d, WeaponType w)
        : name(n), description(d), type(UpgradeType::NEW_WEAPON), weaponType(w), value(0) {}
};

// ============================================================================
// Utility Functions
// ============================================================================

namespace Utils {
    // Random number generation
    inline std::mt19937& GetRNG() {
        static std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
        return rng;
    }
    
    inline float RandomFloat(float min, float max) {
        std::uniform_real_distribution<float> dist(min, max);
        return dist(GetRNG());
    }
    
    inline int RandomInt(int min, int max) {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(GetRNG());
    }
    
    inline glm::vec2 RandomDirection() {
        float angle = RandomFloat(0.0f, 2.0f * glm::pi<float>());
        return glm::vec2(std::cos(angle), std::sin(angle));
    }
    
    inline glm::vec2 RandomPointInCircle(float radius) {
        float r = radius * std::sqrt(RandomFloat(0.0f, 1.0f));
        float theta = RandomFloat(0.0f, 2.0f * glm::pi<float>());
        return glm::vec2(r * std::cos(theta), r * std::sin(theta));
    }
    
    // Vector utilities
    inline float Length(const glm::vec2& v) {
        return std::sqrt(v.x * v.x + v.y * v.y);
    }
    
    inline glm::vec2 Normalize(const glm::vec2& v) {
        float len = Length(v);
        if (len > 0.0001f) {
            return v / len;
        }
        return glm::vec2(0.0f);
    }
    
    inline float Distance(const glm::vec2& a, const glm::vec2& b) {
        return Length(b - a);
    }
    
    inline float Angle(const glm::vec2& v) {
        return std::atan2(v.y, v.x);
    }
    
    inline glm::vec2 RotateVector(const glm::vec2& v, float angle) {
        float c = std::cos(angle);
        float s = std::sin(angle);
        return glm::vec2(v.x * c - v.y * s, v.x * s + v.y * c);
    }
    
    // Collision detection
    inline bool CircleCollision(const glm::vec2& pos1, float r1, const glm::vec2& pos2, float r2) {
        float dx = pos2.x - pos1.x;
        float dy = pos2.y - pos1.y;
        float distSq = dx * dx + dy * dy;
        float radiusSum = r1 + r2;
        return distSq < radiusSum * radiusSum;
    }
    
    inline bool PointInCircle(const glm::vec2& point, const glm::vec2& center, float radius) {
        return Distance(point, center) < radius;
    }
    
    // Lerp and easing
    inline float Lerp(float a, float b, float t) {
        return a + t * (b - a);
    }
    
    inline glm::vec2 Lerp(const glm::vec2& a, const glm::vec2& b, float t) {
        return a + t * (b - a);
    }
    
    inline glm::vec3 Lerp(const glm::vec3& a, const glm::vec3& b, float t) {
        return a + t * (b - a);
    }
    
    inline glm::vec4 Lerp(const glm::vec4& a, const glm::vec4& b, float t) {
        return a + t * (b - a);
    }
    
    inline float EaseOutQuad(float t) {
        return 1.0f - (1.0f - t) * (1.0f - t);
    }
    
    inline float EaseInQuad(float t) {
        return t * t;
    }
    
    inline float EaseInOutQuad(float t) {
        return t < 0.5f ? 2.0f * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 2.0f) / 2.0f;
    }
    
    // Clamp
    template<typename T>
    inline T Clamp(T value, T min, T max) {
        return std::max(min, std::min(max, value));
    }
    
    // File reading
    inline std::string ReadFile(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << filepath << std::endl;
            return "";
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
    
    // Time formatting
    inline std::string FormatTime(float seconds) {
        int mins = static_cast<int>(seconds) / 60;
        int secs = static_cast<int>(seconds) % 60;
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%02d:%02d", mins, secs);
        return std::string(buffer);
    }
}

// ============================================================================
// Color Utilities
// ============================================================================

namespace Colors {
    const glm::vec4 WHITE(1.0f, 1.0f, 1.0f, 1.0f);
    const glm::vec4 BLACK(0.0f, 0.0f, 0.0f, 1.0f);
    const glm::vec4 RED(1.0f, 0.2f, 0.2f, 1.0f);
    const glm::vec4 GREEN(0.2f, 1.0f, 0.2f, 1.0f);
    const glm::vec4 BLUE(0.2f, 0.4f, 1.0f, 1.0f);
    const glm::vec4 YELLOW(1.0f, 1.0f, 0.2f, 1.0f);
    const glm::vec4 CYAN(0.2f, 1.0f, 1.0f, 1.0f);
    const glm::vec4 MAGENTA(1.0f, 0.2f, 1.0f, 1.0f);
    const glm::vec4 ORANGE(1.0f, 0.5f, 0.1f, 1.0f);
    const glm::vec4 PURPLE(0.6f, 0.2f, 0.8f, 1.0f);
    const glm::vec4 PLAYER_BLUE(0.3f, 0.6f, 1.0f, 1.0f);
    const glm::vec4 ENEMY_RED(0.9f, 0.3f, 0.3f, 1.0f);
    const glm::vec4 XP_GREEN(0.4f, 1.0f, 0.4f, 1.0f);
    const glm::vec4 HEALTH_RED(0.8f, 0.2f, 0.2f, 1.0f);
    const glm::vec4 SHIELD_CYAN(0.3f, 0.8f, 1.0f, 0.6f);
}

#endif // UTILS_H
