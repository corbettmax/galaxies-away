#ifndef GAME_H
#define GAME_H

// ============================================================================
// Galaxies Away - Game Manager
// Main game class that manages game state, updates, and rendering
// ============================================================================

#include "utils.h"
#include "renderer.h"
#include "entities.h"
#include "weapons.h"

// ============================================================================
// High Score Entry
// ============================================================================

struct HighScoreEntry {
    float survivalTime;
    int level;
    int enemiesKilled;
    
    HighScoreEntry() : survivalTime(0.0f), level(1), enemiesKilled(0) {}
    HighScoreEntry(float time, int lvl, int kills) : survivalTime(time), level(lvl), enemiesKilled(kills) {}
};

// ============================================================================
// Game Class
// ============================================================================

class Game {
public:
    Game();
    ~Game();
    
    // Initialization
    bool Initialize();
    void Shutdown();
    
    // Main loop
    void Run();
    
    // Callbacks from other systems
    void OnPlayerLevelUp();
    void OnPlayerDeath();
    
    // Accessors
    EntityManager* GetEntityManager() { return &entityManager; }
    WeaponManager* GetWeaponManager() { return &weaponManager; }
    GameState GetState() const { return gameState; }
    float GetGameTime() const { return gameTime; }
    
private:
    // Core loop
    void ProcessInput();
    void Update(float deltaTime);
    void Render();
    
    // State management
    void SetState(GameState newState);
    void UpdateMenuState(float deltaTime);
    void UpdatePlayingState(float deltaTime);
    void UpdateLevelUpState(float deltaTime);
    void UpdatePausedState(float deltaTime);
    void UpdateGameOverState(float deltaTime);
    
    // Game logic
    void StartNewGame();
    void SpawnEnemies(float deltaTime);
    void UpdateDifficulty();
    glm::vec2 GetRandomSpawnPosition() const;
    
    // Level up menu
    void GenerateLevelUpChoices();
    void ApplyUpgradeChoice(int choiceIndex);
    
    // Rendering
    void RenderGame();
    void RenderUI();
    void RenderMenu();
    void RenderLevelUpMenu();
    void RenderPauseMenu();
    void RenderGameOver();
    void RenderHUD();
    
    // High scores
    void LoadHighScores();
    void SaveHighScores();
    void AddHighScore(const HighScoreEntry& entry);
    
    // Input callbacks (GLFW)
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    
    // Window
    GLFWwindow* window;
    int windowWidth, windowHeight;
    
    // Renderer
    Renderer renderer;
    
    // Game systems
    EntityManager entityManager;
    WeaponManager weaponManager;
    
    // Game state
    GameState gameState;
    GameState previousState;
    
    // Timing
    float gameTime;         // Total time survived
    float deltaTime;
    float lastFrameTime;
    
    // Enemy spawning
    float spawnTimer;
    float spawnRate;
    float difficultyTimer;
    int difficultyLevel;
    
    // Boss spawning
    float bossTimer;
    float bossInterval;
    int bossesDefeated;
    
    // Stats
    int enemiesKilled;
    int totalXPCollected;
    
    // Level up menu
    std::vector<UpgradeChoice> currentChoices;
    int selectedChoice;
    int numChoices;
    
    // Input state
    bool keys[GLFW_KEY_LAST + 1];
    bool keysPressed[GLFW_KEY_LAST + 1];
    glm::vec2 mousePos;
    bool mouseButtons[GLFW_MOUSE_BUTTON_LAST + 1];
    bool mouseButtonsPressed[GLFW_MOUSE_BUTTON_LAST + 1];
    
    // High scores
    std::vector<HighScoreEntry> highScores;
    static const int MAX_HIGH_SCORES = 10;
    
    // Debug
    bool showDebugInfo;
};

// Global game instance pointer (for GLFW callbacks)
extern Game* g_Game;

#endif // GAME_H
