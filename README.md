# Galaxies Away

A space-themed roguelike shoot'em up game inspired by Vampire Survivors, built with C++ and OpenGL.

![Galaxies Away](assets/screenshot_placeholder.png)

## Features

### Gameplay
- **Auto-shooting**: Your ship automatically fires at nearby enemies
- **Wave-based survival**: Endless waves of enemies with increasing difficulty
- **8-directional movement**: Smooth WASD/Arrow key controls
- **XP and leveling system**: Collect green orbs from defeated enemies
- **Power-up selection**: Choose from 4 random upgrades on each level up
- **Boss battles**: Powerful boss enemies spawn every 60 seconds

### Weapons (6 types)
1. **Laser** - Fast-firing beam that can pierce multiple enemies
2. **Missiles** - Homing projectiles that seek out targets
3. **Orbital Drones** - Satellites that orbit you and damage enemies on contact
4. **Energy Shield** - Protective aura that damages nearby enemies
5. **Plasma Bombs** - AOE explosions that hit all enemies in radius
6. **Spread Shot** - Shotgun-style spread of projectiles

### Enemy Types
1. **Basic Fighters** - Standard enemies that chase the player
2. **Tank Units** - Slow but heavily armored enemies
3. **Fast Interceptors** - Quick enemies that sometimes dodge your attacks
4. **Boss Ships** - Large powerful enemies with special attacks

### Visual Effects
- Dynamic starfield background with twinkling stars
- Particle effects for explosions, hit sparks, and trails
- Screen shake for impactful moments
- Health and XP bar UI
- Level-up menu with upgrade choices

## Building the Game

### Dependencies

#### Linux (Debian/Ubuntu)
```bash
# Install dependencies
make install-deps-linux
# Or manually:
sudo apt-get install libglfw3-dev libglm-dev libfreetype6-dev build-essential
```

#### macOS
```bash
# Install dependencies
make install-deps-macos
# Or manually:
brew install glfw glm freetype
```

#### Windows (MinGW)
1. Download and install MSYS2 or MinGW-w64
2. Install GLFW and GLM through your package manager
3. Modify the Makefile to use Windows paths and libraries

### Compilation

```bash
# Build the game
make

# Build with debug symbols
make debug

# Build and run
make run

# Clean build files
make clean
```

The executable will be created as `galaxies_away` in the project root.

## Controls

| Key | Action |
|-----|--------|
| W / ↑ | Move Up |
| S / ↓ | Move Down |
| A / ← | Move Left |
| D / → | Move Right |
| ESC | Pause/Resume |
| SPACE/ENTER | Confirm selection |
| 1-4 | Quick select upgrade |
| F3 | Toggle debug info |
| Q | Quit to menu (when paused/game over) |

## Project Structure

```
galaxies-away/
├── src/
│   ├── main.cpp        # Entry point
│   ├── game.cpp/h      # Main game logic and state management
│   ├── renderer.cpp/h  # OpenGL rendering system
│   ├── entities.cpp/h  # Player, enemies, projectiles, particles
│   ├── weapons.cpp/h   # Weapon system and upgrades
│   └── utils.cpp/h     # Utilities, constants, math helpers
├── shaders/
│   ├── vertex.glsl            # Sprite vertex shader
│   ├── fragment.glsl          # Sprite fragment shader
│   ├── particle_vertex.glsl   # Particle vertex shader
│   ├── particle_fragment.glsl # Particle fragment shader
│   ├── text_vertex.glsl       # Text vertex shader
│   └── text_fragment.glsl     # Text fragment shader
├── assets/
│   └── textures/       # Placeholder for textures
├── Makefile
├── README.md
└── LICENSE
```

## Technical Details

### Graphics
- OpenGL 3.3 Core Profile
- Custom shader-based sprite rendering
- Particle system with batch rendering
- Camera system for smooth scrolling
- FreeType-based text rendering with font atlases

### Architecture
- Entity-component inspired design
- Clean separation of concerns (rendering, logic, input)
- Efficient collision detection
- State machine for game flow

### Performance
- Particle pooling to avoid allocations
- Batch rendering for particles
- Entity cleanup to remove dead objects
- Delta time based updates


## Known Limitations

1. No audio implementation yet
2. Collision detection is simple circle-based
3. No sprite textures (uses colored shapes)
4. Single save file for high scores

## License

MIT License - See LICENSE file for details.

## Credits

- Game Design & Programming: Max Corbett
- Inspired by: Vampire Survivors, Geometry Wars
- Built with: C++17, OpenGL 3.3, GLFW, GLM

---

**Enjoy the game and survive as long as you can!** 🚀
