// ============================================================================
// Galaxies Away - Main Entry Point
// A Space-Themed Roguelike Shoot'em Up Game
// 
// Similar to Vampire Survivors, featuring:
// - Auto-shooting mechanics
// - Wave-based survival with increasing difficulty
// - XP collection and level-up system
// - Multiple weapon types and upgrades
// - Particle effects and smooth graphics
// 
// Built with C++17 and OpenGL 3.3
// ============================================================================

#include "game.h"

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    
    std::cout << "========================================" << std::endl;
    std::cout << "       GALAXIES AWAY" << std::endl;
    std::cout << "  Space Roguelike Shoot'em Up" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    
    // Create and initialize game
    Game game;
    
    if (!game.Initialize()) {
        std::cerr << "Failed to initialize game!" << std::endl;
        return -1;
    }
    
    // Run main game loop
    game.Run();
    
    // Cleanup
    game.Shutdown();
    
    std::cout << std::endl;
    std::cout << "Thanks for playing Galaxies Away!" << std::endl;
    
    return 0;
}
