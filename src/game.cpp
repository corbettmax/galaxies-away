// ============================================================================
// Galaxies Away - Game Manager Implementation
// ============================================================================

#include "game.h"
#include <algorithm>

// Global game instance for callbacks
Game* g_Game = nullptr;

// ============================================================================
// Constructor / Destructor
// ============================================================================

Game::Game()
    : window(nullptr)
    , windowWidth(Constants::WINDOW_WIDTH)
    , windowHeight(Constants::WINDOW_HEIGHT)
    , gameState(GameState::MENU)
    , previousState(GameState::MENU)
    , gameTime(0.0f)
    , deltaTime(0.0f)
    , lastFrameTime(0.0f)
    , spawnTimer(0.0f)
    , spawnRate(Constants::BASE_SPAWN_RATE)
    , difficultyTimer(0.0f)
    , difficultyLevel(1)
    , bossTimer(0.0f)
    , bossInterval(60.0f)
    , bossesDefeated(0)
    , enemiesKilled(0)
    , totalXPCollected(0)
    , selectedChoice(0)
    , numChoices(4)
    , showDebugInfo(false)
{
    std::memset(keys, 0, sizeof(keys));
    std::memset(keysPressed, 0, sizeof(keysPressed));
    std::memset(mouseButtons, 0, sizeof(mouseButtons));
    std::memset(mouseButtonsPressed, 0, sizeof(mouseButtonsPressed));
    mousePos = glm::vec2(0.0f);
}

Game::~Game() {
    Shutdown();
}

// ============================================================================
// Initialization
// ============================================================================

bool Game::Initialize() {
    g_Game = this;
    
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }
    
    // Configure GLFW for OpenGL 3.3 Core
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    
    // Create window
    window = glfwCreateWindow(windowWidth, windowHeight, Constants::WINDOW_TITLE, nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    
    // Set callbacks
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetCursorPosCallback(window, CursorPosCallback);
    
    // Initialize renderer
    if (!renderer.Initialize(windowWidth, windowHeight)) {
        std::cerr << "Failed to initialize renderer" << std::endl;
        return false;
    }
    
    g_Renderer = &renderer;
    
    // Load textures
    if (!enemyTexture.LoadFromFile("assets/textures/tie-fighter.png")) {
        std::cerr << "Warning: Failed to load enemy texture, using solid colors" << std::endl;
    }
    
    // Load high scores
    LoadHighScores();
    
    std::cout << "Game initialized successfully!" << std::endl;
    std::cout << "Press SPACE or ENTER to start" << std::endl;
    
    return true;
}

void Game::Shutdown() {
    SaveHighScores();
    
    renderer.Shutdown();
    
    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    
    glfwTerminate();
    
    g_Game = nullptr;
    g_Renderer = nullptr;
}

// ============================================================================
// Main Loop
// ============================================================================

void Game::Run() {
    lastFrameTime = static_cast<float>(glfwGetTime());
    
    while (!glfwWindowShouldClose(window)) {
        // Calculate delta time
        float currentTime = static_cast<float>(glfwGetTime());
        deltaTime = currentTime - lastFrameTime;
        lastFrameTime = currentTime;
        
        // Cap delta time to prevent physics issues
        deltaTime = std::min(deltaTime, 0.1f);
        
        // Process input
        ProcessInput();
        
        // Update
        Update(deltaTime);
        
        // Render
        Render();
        
        // Swap buffers and poll events
        glfwSwapBuffers(window);
        
        // Clear pressed keys/buttons for next frame
        std::memset(keysPressed, 0, sizeof(keysPressed));
        std::memset(mouseButtonsPressed, 0, sizeof(mouseButtonsPressed));
        
        glfwPollEvents();
    }
}

// ============================================================================
// Input Processing
// ============================================================================

void Game::ProcessInput() {
    // ESC to pause/unpause or return to menu
    if (keysPressed[GLFW_KEY_ESCAPE]) {
        if (gameState == GameState::PLAYING) {
            SetState(GameState::PAUSED);
        } else if (gameState == GameState::PAUSED) {
            SetState(GameState::PLAYING);
        } else if (gameState == GameState::LEVEL_UP || gameState == GameState::GAME_OVER) {
            SetState(GameState::MENU);
        }
    }
    
    // Debug toggle
    if (keysPressed[GLFW_KEY_F3]) {
        showDebugInfo = !showDebugInfo;
    }
    
    // State-specific input
    switch (gameState) {
        case GameState::MENU:
            if (keysPressed[GLFW_KEY_SPACE] || keysPressed[GLFW_KEY_ENTER]) {
                StartNewGame();
            }
            break;
            
        case GameState::PLAYING:
            // Movement input for player
            if (entityManager.player) {
                glm::vec2 input(0.0f);
                if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP]) input.y -= 1.0f;
                if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN]) input.y += 1.0f;
                if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT]) input.x -= 1.0f;
                if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT]) input.x += 1.0f;
                entityManager.player->moveInput = input;
            }
            break;
            
        case GameState::LEVEL_UP:
            // Navigate choices
            if (keysPressed[GLFW_KEY_W] || keysPressed[GLFW_KEY_UP]) {
                selectedChoice = (selectedChoice - 1 + numChoices) % numChoices;
            }
            if (keysPressed[GLFW_KEY_S] || keysPressed[GLFW_KEY_DOWN]) {
                selectedChoice = (selectedChoice + 1) % numChoices;
            }
            // Select choice with number keys
            for (int i = 0; i < numChoices; ++i) {
                if (keysPressed[GLFW_KEY_1 + i]) {
                    ApplyUpgradeChoice(i);
                }
            }
            // Confirm selection
            if (keysPressed[GLFW_KEY_SPACE] || keysPressed[GLFW_KEY_ENTER]) {
                ApplyUpgradeChoice(selectedChoice);
            }
            break;
            
        case GameState::PAUSED:
            if (keysPressed[GLFW_KEY_SPACE] || keysPressed[GLFW_KEY_ENTER]) {
                SetState(GameState::PLAYING);
            }
            if (keysPressed[GLFW_KEY_Q]) {
                SetState(GameState::MENU);
            }
            break;
            
        case GameState::GAME_OVER:
            if (keysPressed[GLFW_KEY_SPACE] || keysPressed[GLFW_KEY_ENTER]) {
                StartNewGame();
            }
            if (keysPressed[GLFW_KEY_Q]) {
                SetState(GameState::MENU);
            }
            break;
    }
}

// ============================================================================
// Update
// ============================================================================

void Game::Update(float dt) {
    switch (gameState) {
        case GameState::MENU:
            UpdateMenuState(dt);
            break;
        case GameState::PLAYING:
            UpdatePlayingState(dt);
            break;
        case GameState::LEVEL_UP:
            UpdateLevelUpState(dt);
            break;
        case GameState::PAUSED:
            UpdatePausedState(dt);
            break;
        case GameState::GAME_OVER:
            UpdateGameOverState(dt);
            break;
    }
    
    // Always update screen shake
    renderer.UpdateScreenShake(dt);
}

void Game::UpdateMenuState(float dt) {
    (void)dt;
    // Could animate menu elements here
}

void Game::UpdatePlayingState(float dt) {
    // Update game time
    gameTime += dt;
    
    // Update difficulty
    UpdateDifficulty();
    
    // Spawn enemies
    SpawnEnemies(dt);
    
    // Update all entities
    entityManager.Update(dt, this);
    
    // Update weapons
    weaponManager.Update(dt, this);
    weaponManager.UpdateAllStats(entityManager.player.get());
    
    // Update camera to follow player
    if (entityManager.player) {
        renderer.SetCameraPosition(entityManager.player->position);
    }
    
    // Check for player death
    if (entityManager.player && entityManager.player->health <= 0.0f) {
        OnPlayerDeath();
    }
}

void Game::UpdateLevelUpState(float dt) {
    (void)dt;
    // Level up menu is mostly static
    // Could add animations here
}

void Game::UpdatePausedState(float dt) {
    (void)dt;
    // Paused - no updates
}

void Game::UpdateGameOverState(float dt) {
    (void)dt;
    // Could animate game over screen
}

// ============================================================================
// Rendering
// ============================================================================

void Game::Render() {
    renderer.BeginFrame();
    
    switch (gameState) {
        case GameState::MENU:
            RenderMenu();
            break;
        case GameState::PLAYING:
            RenderGame();
            RenderHUD();
            break;
        case GameState::LEVEL_UP:
            RenderGame();  // Show game in background
            RenderLevelUpMenu();
            break;
        case GameState::PAUSED:
            RenderGame();
            RenderPauseMenu();
            break;
        case GameState::GAME_OVER:
            RenderGame();
            RenderGameOver();
            break;
    }
    
    // Debug info
    if (showDebugInfo) {
        std::string debugText = "FPS: " + std::to_string(static_cast<int>(1.0f / deltaTime));
        debugText += "\nEntities: " + std::to_string(entityManager.GetEnemyCount());
        debugText += "\nProjectiles: " + std::to_string(entityManager.GetProjectileCount());
        debugText += "\nParticles: " + std::to_string(entityManager.particleSystem.GetActiveCount());
        renderer.DrawText(debugText, glm::vec2(10, windowHeight - 80), 0.8f, Colors::WHITE);
    }
    
    renderer.EndFrame();
}

void Game::RenderGame() {
    // Draw starfield background
    renderer.DrawStarfield(static_cast<float>(glfwGetTime()));
    
    // Render all entities
    entityManager.Render(&renderer);
    
    // Render weapon visuals (orbitals, shields, etc.)
    if (entityManager.player) {
        weaponManager.Render(&renderer, entityManager.player->position);
    }
}

void Game::RenderHUD() {
    float padding = 20.0f;
    float barHeight = 20.0f;
    float barWidth = 200.0f;
    
    // Health bar
    if (entityManager.player) {
        Player* player = entityManager.player.get();
        
        glm::vec2 healthBarPos(padding, padding);
        float healthPercent = player->health / player->maxHealth;
        renderer.DrawProgressBar(healthBarPos, glm::vec2(barWidth, barHeight), healthPercent, 
                                Colors::HEALTH_RED, glm::vec4(0.2f, 0.2f, 0.2f, 0.8f));
        
        // Health text
        std::string healthText = "HP: " + std::to_string(static_cast<int>(player->health)) + 
                                "/" + std::to_string(static_cast<int>(player->maxHealth));
        renderer.DrawText(healthText, glm::vec2(padding + barWidth + 10, padding + 2), 0.9f, Colors::WHITE);
        
        // XP bar
        glm::vec2 xpBarPos(padding, padding + barHeight + 10);
        float xpPercent = static_cast<float>(player->experience) / player->experienceToNextLevel;
        renderer.DrawProgressBar(xpBarPos, glm::vec2(barWidth, barHeight * 0.7f), xpPercent,
                                Colors::XP_GREEN, glm::vec4(0.2f, 0.2f, 0.2f, 0.8f));
        
        // Level text
        std::string levelText = "Lv." + std::to_string(player->level);
        renderer.DrawText(levelText, glm::vec2(padding + barWidth + 10, padding + barHeight + 10), 0.9f, Colors::YELLOW);
    }
    
    // Timer (top center)
    std::string timeText = Utils::FormatTime(gameTime);
    float timeWidth = timeText.length() * 12.0f;
    renderer.DrawText(timeText, glm::vec2((windowWidth - timeWidth) / 2, padding), 1.2f, Colors::WHITE);
    
    // Kill count (top right)
    std::string killText = "Kills: " + std::to_string(enemiesKilled);
    float killWidth = killText.length() * 10.0f;
    renderer.DrawText(killText, glm::vec2(windowWidth - killWidth - padding, padding), 0.9f, Colors::WHITE);
    
    // Weapon info (bottom left)
    float weaponY = windowHeight - padding - 20.0f;
    for (int i = weaponManager.GetWeaponCount() - 1; i >= 0; --i) {
        std::string weaponDesc = weaponManager.weapons[i]->GetDescription();
        renderer.DrawText(weaponDesc, glm::vec2(padding, weaponY), 0.7f, Colors::CYAN);
        weaponY -= 18.0f;
    }
}

void Game::RenderMenu() {
    // Title
    std::string title = "GALAXIES AWAY";
    float titleWidth = title.length() * 20.0f;
    renderer.DrawText(title, glm::vec2((windowWidth - titleWidth) / 2, 150), 2.0f, Colors::CYAN);
    
    // Subtitle
    std::string subtitle = "Space Roguelike Survival";
    float subWidth = subtitle.length() * 10.0f;
    renderer.DrawText(subtitle, glm::vec2((windowWidth - subWidth) / 2, 210), 1.0f, Colors::WHITE);
    
    // Instructions
    std::string startText = "Press SPACE or ENTER to Start";
    float startWidth = startText.length() * 10.0f;
    
    // Pulsing effect
    float pulse = 0.7f + 0.3f * std::sin(static_cast<float>(glfwGetTime()) * 3.0f);
    glm::vec4 startColor = glm::vec4(1.0f, 1.0f, 1.0f, pulse);
    renderer.DrawText(startText, glm::vec2((windowWidth - startWidth) / 2, 350), 1.0f, startColor);
    
    // Controls
    renderer.DrawText("CONTROLS:", glm::vec2(100, 450), 1.0f, Colors::YELLOW);
    renderer.DrawText("WASD / Arrow Keys - Move", glm::vec2(100, 480), 0.8f, Colors::WHITE);
    renderer.DrawText("Weapons fire automatically", glm::vec2(100, 510), 0.8f, Colors::WHITE);
    renderer.DrawText("Collect green orbs for XP", glm::vec2(100, 540), 0.8f, Colors::WHITE);
    renderer.DrawText("ESC - Pause", glm::vec2(100, 570), 0.8f, Colors::WHITE);
    
    // High scores
    if (!highScores.empty()) {
        renderer.DrawText("HIGH SCORES:", glm::vec2(windowWidth - 300, 450), 1.0f, Colors::YELLOW);
        for (size_t i = 0; i < std::min(highScores.size(), (size_t)5); ++i) {
            std::string scoreText = std::to_string(i + 1) + ". " + 
                                   Utils::FormatTime(highScores[i].survivalTime) + 
                                   " Lv." + std::to_string(highScores[i].level);
            renderer.DrawText(scoreText, glm::vec2(windowWidth - 300, 480 + i * 25), 0.8f, Colors::WHITE);
        }
    }
    
    // Animated stars in background
    renderer.DrawStarfield(static_cast<float>(glfwGetTime()));
}

void Game::RenderLevelUpMenu() {
    // Dim background
    renderer.DrawRect(glm::vec2(0, 0), glm::vec2(windowWidth, windowHeight), glm::vec4(0.0f, 0.0f, 0.0f, 0.7f));
    
    // Title
    std::string title = "LEVEL UP!";
    float titleWidth = title.length() * 20.0f;
    renderer.DrawText(title, glm::vec2((windowWidth - titleWidth) / 2, 100), 2.0f, Colors::YELLOW);
    
    // Level info
    if (entityManager.player) {
        std::string levelText = "You reached Level " + std::to_string(entityManager.player->level);
        float levelWidth = levelText.length() * 10.0f;
        renderer.DrawText(levelText, glm::vec2((windowWidth - levelWidth) / 2, 160), 1.0f, Colors::WHITE);
    }
    
    // Choices
    float choiceY = 230.0f;
    float choiceWidth = 400.0f;
    float choiceHeight = 70.0f;
    float choiceX = (windowWidth - choiceWidth) / 2;
    
    for (int i = 0; i < numChoices && i < static_cast<int>(currentChoices.size()); ++i) {
        const UpgradeChoice& choice = currentChoices[i];
        
        // Background
        glm::vec4 bgColor = (i == selectedChoice) ? glm::vec4(0.3f, 0.5f, 0.8f, 0.8f) : glm::vec4(0.2f, 0.2f, 0.3f, 0.8f);
        renderer.DrawRect(glm::vec2(choiceX, choiceY), glm::vec2(choiceWidth, choiceHeight), bgColor);
        
        // Selection indicator
        if (i == selectedChoice) {
            renderer.DrawRectOutline(glm::vec2(choiceX, choiceY), glm::vec2(choiceWidth, choiceHeight), 3.0f, Colors::CYAN);
        }
        
        // Choice number
        std::string numText = std::to_string(i + 1) + ".";
        renderer.DrawText(numText, glm::vec2(choiceX + 15, choiceY + 10), 1.0f, Colors::YELLOW);
        
        // Choice name
        renderer.DrawText(choice.name, glm::vec2(choiceX + 50, choiceY + 10), 1.0f, Colors::WHITE);
        
        // Description
        renderer.DrawText(choice.description, glm::vec2(choiceX + 50, choiceY + 40), 0.7f, glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));
        
        choiceY += choiceHeight + 15.0f;
    }
    
    // Instructions
    std::string instructions = "Use W/S or 1-4 to select, SPACE/ENTER to confirm";
    float instrWidth = instructions.length() * 8.0f;
    renderer.DrawText(instructions, glm::vec2((windowWidth - instrWidth) / 2, windowHeight - 50), 0.8f, Colors::WHITE);
}

void Game::RenderPauseMenu() {
    // Dim background
    renderer.DrawRect(glm::vec2(0, 0), glm::vec2(windowWidth, windowHeight), glm::vec4(0.0f, 0.0f, 0.0f, 0.6f));
    
    // Pause text
    std::string pauseText = "PAUSED";
    float pauseWidth = pauseText.length() * 25.0f;
    renderer.DrawText(pauseText, glm::vec2((windowWidth - pauseWidth) / 2, 250), 2.5f, Colors::WHITE);
    
    // Options
    std::string resumeText = "Press SPACE or ESC to Resume";
    float resumeWidth = resumeText.length() * 10.0f;
    renderer.DrawText(resumeText, glm::vec2((windowWidth - resumeWidth) / 2, 350), 1.0f, Colors::CYAN);
    
    std::string quitText = "Press Q to Quit to Menu";
    float quitWidth = quitText.length() * 10.0f;
    renderer.DrawText(quitText, glm::vec2((windowWidth - quitWidth) / 2, 390), 1.0f, Colors::WHITE);
}

void Game::RenderGameOver() {
    // Dim background
    renderer.DrawRect(glm::vec2(0, 0), glm::vec2(windowWidth, windowHeight), glm::vec4(0.0f, 0.0f, 0.0f, 0.8f));
    
    // Game over text
    std::string gameOverText = "GAME OVER";
    float goWidth = gameOverText.length() * 25.0f;
    renderer.DrawText(gameOverText, glm::vec2((windowWidth - goWidth) / 2, 150), 2.5f, Colors::RED);
    
    // Stats
    float statY = 260.0f;
    float statX = windowWidth / 2 - 150.0f;
    
    renderer.DrawText("FINAL STATS", glm::vec2(statX, statY), 1.2f, Colors::YELLOW);
    statY += 40.0f;
    
    std::string timeText = "Survival Time: " + Utils::FormatTime(gameTime);
    renderer.DrawText(timeText, glm::vec2(statX, statY), 1.0f, Colors::WHITE);
    statY += 30.0f;
    
    if (entityManager.player) {
        std::string levelText = "Final Level: " + std::to_string(entityManager.player->level);
        renderer.DrawText(levelText, glm::vec2(statX, statY), 1.0f, Colors::WHITE);
        statY += 30.0f;
    }
    
    std::string killText = "Enemies Defeated: " + std::to_string(enemiesKilled);
    renderer.DrawText(killText, glm::vec2(statX, statY), 1.0f, Colors::WHITE);
    statY += 30.0f;
    
    std::string weaponText = "Weapons Acquired: " + std::to_string(weaponManager.GetWeaponCount());
    renderer.DrawText(weaponText, glm::vec2(statX, statY), 1.0f, Colors::WHITE);
    statY += 50.0f;
    
    // Options
    std::string retryText = "Press SPACE to Try Again";
    float retryWidth = retryText.length() * 10.0f;
    float pulse = 0.7f + 0.3f * std::sin(static_cast<float>(glfwGetTime()) * 3.0f);
    renderer.DrawText(retryText, glm::vec2((windowWidth - retryWidth) / 2, statY), 1.0f, 
                      glm::vec4(1.0f, 1.0f, 1.0f, pulse));
    
    std::string menuText = "Press Q for Menu";
    float menuWidth = menuText.length() * 10.0f;
    renderer.DrawText(menuText, glm::vec2((windowWidth - menuWidth) / 2, statY + 40), 0.9f, Colors::WHITE);
}

// ============================================================================
// State Management
// ============================================================================

void Game::SetState(GameState newState) {
    previousState = gameState;
    gameState = newState;
}

void Game::StartNewGame() {
    // Reset game state
    gameTime = 0.0f;
    enemiesKilled = 0;
    totalXPCollected = 0;
    difficultyLevel = 1;
    spawnRate = Constants::BASE_SPAWN_RATE;
    bossTimer = 0.0f;
    bossesDefeated = 0;
    
    // Clear entities
    entityManager.Clear();
    
    // Reset weapons
    weaponManager = WeaponManager();
    
    // Spawn player at center
    entityManager.SpawnPlayer(glm::vec2(0.0f));
    
    // Set camera
    renderer.SetCameraPosition(glm::vec2(0.0f));
    
    SetState(GameState::PLAYING);
    
    std::cout << "New game started!" << std::endl;
}

// ============================================================================
// Enemy Spawning
// ============================================================================

void Game::SpawnEnemies(float dt) {
    spawnTimer -= dt;
    
    if (spawnTimer <= 0.0f) {
        spawnTimer = spawnRate;
        
        // Determine enemy type based on difficulty and randomness
        EntityType enemyType = EntityType::ENEMY_BASIC;
        float roll = Utils::RandomFloat(0.0f, 1.0f);
        
        if (difficultyLevel >= 3 && roll < 0.15f) {
            enemyType = EntityType::ENEMY_FAST;
        } else if (difficultyLevel >= 2 && roll < 0.25f) {
            enemyType = EntityType::ENEMY_TANK;
        }
        
        // Spawn position
        glm::vec2 spawnPos = GetRandomSpawnPosition();
        
        // Spawn 1-3 enemies based on difficulty
        int spawnCount = 1 + (difficultyLevel / 3);
        spawnCount = std::min(spawnCount, 5);
        
        for (int i = 0; i < spawnCount; ++i) {
            glm::vec2 offset = Utils::RandomPointInCircle(50.0f);
            entityManager.SpawnEnemy(enemyType, spawnPos + offset);
        }
    }
    
    // Boss spawning
    bossTimer += dt;
    if (bossTimer >= bossInterval) {
        bossTimer = 0.0f;
        
        glm::vec2 bossPos = GetRandomSpawnPosition();
        BossEnemy* boss = static_cast<BossEnemy*>(entityManager.SpawnEnemy(EntityType::ENEMY_BOSS, bossPos));
        
        // Scale boss based on how many have been defeated
        boss->maxHealth *= (1.0f + bossesDefeated * 0.5f);
        boss->health = boss->maxHealth;
        boss->damage *= (1.0f + bossesDefeated * 0.2f);
        
        // Screen shake for boss spawn
        renderer.SetScreenShake(10.0f, 0.5f);
        
        std::cout << "Boss spawned!" << std::endl;
    }
}

void Game::UpdateDifficulty() {
    // Increase difficulty every 30 seconds
    difficultyTimer += deltaTime;
    if (difficultyTimer >= 30.0f) {
        difficultyTimer = 0.0f;
        difficultyLevel++;
        
        // Increase spawn rate
        spawnRate = std::max(Constants::MIN_SPAWN_RATE, spawnRate * 0.9f);
        
        std::cout << "Difficulty increased to level " << difficultyLevel << std::endl;
    }
}

glm::vec2 Game::GetRandomSpawnPosition() const {
    if (!entityManager.player) {
        return glm::vec2(0.0f);
    }
    
    glm::vec2 playerPos = entityManager.player->position;
    
    // Spawn at random angle from player
    float angle = Utils::RandomFloat(0.0f, glm::two_pi<float>());
    float distance = Utils::RandomFloat(Constants::SPAWN_DISTANCE_MIN, Constants::SPAWN_DISTANCE_MAX);
    
    glm::vec2 spawnPos = playerPos + glm::vec2(std::cos(angle), std::sin(angle)) * distance;
    
    // Clamp to world bounds
    float halfWidth = Constants::WORLD_WIDTH * 0.5f;
    float halfHeight = Constants::WORLD_HEIGHT * 0.5f;
    spawnPos.x = Utils::Clamp(spawnPos.x, -halfWidth + 50.0f, halfWidth - 50.0f);
    spawnPos.y = Utils::Clamp(spawnPos.y, -halfHeight + 50.0f, halfHeight - 50.0f);
    
    return spawnPos;
}

// ============================================================================
// Level Up
// ============================================================================

void Game::OnPlayerLevelUp() {
    GenerateLevelUpChoices();
    selectedChoice = 0;
    SetState(GameState::LEVEL_UP);
    
    // Play level up sound effect here if implemented
    std::cout << "Level up! Now level " << entityManager.player->level << std::endl;
}

void Game::GenerateLevelUpChoices() {
    currentChoices.clear();
    
    // Get all available upgrades
    std::vector<UpgradeChoice> allChoices = weaponManager.GetAvailableUpgrades();
    
    // Shuffle
    std::shuffle(allChoices.begin(), allChoices.end(), Utils::GetRNG());
    
    // Take first numChoices
    for (int i = 0; i < numChoices && i < static_cast<int>(allChoices.size()); ++i) {
        currentChoices.push_back(allChoices[i]);
    }
}

void Game::ApplyUpgradeChoice(int choiceIndex) {
    if (choiceIndex < 0 || choiceIndex >= static_cast<int>(currentChoices.size())) {
        return;
    }
    
    const UpgradeChoice& choice = currentChoices[choiceIndex];
    
    if (choice.type == UpgradeType::NEW_WEAPON) {
        if (choice.value < 0) {
            // This is a weapon upgrade
            weaponManager.UpgradeWeapon(choice.weaponType);
        } else {
            // This is a new weapon
            weaponManager.AddWeapon(choice.weaponType);
        }
    } else {
        // Apply stat upgrade to player
        if (entityManager.player) {
            entityManager.player->ApplyUpgrade(choice);
        }
    }
    
    // Update weapon stats
    weaponManager.UpdateAllStats(entityManager.player.get());
    
    // Return to playing
    SetState(GameState::PLAYING);
}

// ============================================================================
// Player Death
// ============================================================================

void Game::OnPlayerDeath() {
    // Add to high scores
    HighScoreEntry entry(gameTime, entityManager.player ? entityManager.player->level : 1, enemiesKilled);
    AddHighScore(entry);
    
    // Big explosion effect
    if (entityManager.player) {
        entityManager.particleSystem.SpawnExplosion(entityManager.player->position, Colors::PLAYER_BLUE, 50, 300.0f);
    }
    
    renderer.SetScreenShake(15.0f, 0.5f);
    
    SetState(GameState::GAME_OVER);
    
    std::cout << "Game over! Survived for " << Utils::FormatTime(gameTime) << std::endl;
}

// ============================================================================
// High Scores
// ============================================================================

void Game::LoadHighScores() {
    std::ifstream file("highscores.dat");
    if (!file.is_open()) return;
    
    highScores.clear();
    HighScoreEntry entry;
    while (file >> entry.survivalTime >> entry.level >> entry.enemiesKilled) {
        highScores.push_back(entry);
    }
}

void Game::SaveHighScores() {
    std::ofstream file("highscores.dat");
    if (!file.is_open()) return;
    
    for (const auto& entry : highScores) {
        file << entry.survivalTime << " " << entry.level << " " << entry.enemiesKilled << "\n";
    }
}

void Game::AddHighScore(const HighScoreEntry& entry) {
    highScores.push_back(entry);
    
    // Sort by survival time (descending)
    std::sort(highScores.begin(), highScores.end(),
              [](const HighScoreEntry& a, const HighScoreEntry& b) {
                  return a.survivalTime > b.survivalTime;
              });
    
    // Keep only top scores
    if (highScores.size() > MAX_HIGH_SCORES) {
        highScores.resize(MAX_HIGH_SCORES);
    }
    
    SaveHighScores();
}

// ============================================================================
// GLFW Callbacks
// ============================================================================

void Game::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    (void)window;
    (void)scancode;
    (void)mods;
    
    if (!g_Game) return;
    
    if (key >= 0 && key <= GLFW_KEY_LAST) {
        if (action == GLFW_PRESS) {
            g_Game->keys[key] = true;
            g_Game->keysPressed[key] = true;
        } else if (action == GLFW_RELEASE) {
            g_Game->keys[key] = false;
        }
    }
}

void Game::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    (void)window;
    (void)mods;
    
    if (!g_Game) return;
    
    if (button >= 0 && button <= GLFW_MOUSE_BUTTON_LAST) {
        if (action == GLFW_PRESS) {
            g_Game->mouseButtons[button] = true;
            g_Game->mouseButtonsPressed[button] = true;
        } else if (action == GLFW_RELEASE) {
            g_Game->mouseButtons[button] = false;
        }
    }
}

void Game::CursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    (void)window;
    
    if (!g_Game) return;
    
    g_Game->mousePos = glm::vec2(static_cast<float>(xpos), static_cast<float>(ypos));
}
