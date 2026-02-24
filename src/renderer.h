#ifndef RENDERER_H
#define RENDERER_H

// ============================================================================
// Galaxies Away - OpenGL Renderer
// Handles all graphics rendering: sprites, particles, UI, and background
// ============================================================================

#include "utils.h"
#include <map>

// Include GLFW which brings in OpenGL headers
// GL_GLEXT_PROTOTYPES enables function prototypes for OpenGL extensions
#define GL_GLEXT_PROTOTYPES
#include <GLFW/glfw3.h>

// Include FreeType for text rendering
#include <ft2build.h>
#include FT_FREETYPE_H

// ============================================================================
// Vertex Structure for Sprite Rendering
// ============================================================================

struct Vertex {
    glm::vec2 position;
    glm::vec2 texCoord;
    glm::vec4 color;
    
    Vertex() : position(0.0f), texCoord(0.0f), color(1.0f) {}
    Vertex(const glm::vec2& pos, const glm::vec2& tex, const glm::vec4& col)
        : position(pos), texCoord(tex), color(col) {}
};

// ============================================================================
// Shader Program
// ============================================================================

class Shader {
public:
    GLuint programID;
    
    Shader() : programID(0) {}
    ~Shader();
    
    bool LoadFromStrings(const std::string& vertexSource, const std::string& fragmentSource);
    bool LoadFromFiles(const std::string& vertexPath, const std::string& fragmentPath);
    
    void Use() const;
    void SetInt(const std::string& name, int value) const;
    void SetFloat(const std::string& name, float value) const;
    void SetVec2(const std::string& name, const glm::vec2& value) const;
    void SetVec3(const std::string& name, const glm::vec3& value) const;
    void SetVec4(const std::string& name, const glm::vec4& value) const;
    void SetMat4(const std::string& name, const glm::mat4& value) const;
    
private:
    GLuint CompileShader(GLenum type, const std::string& source);
    bool CheckCompileErrors(GLuint shader, const std::string& type);
};

// ============================================================================
// Texture
// ============================================================================

class Texture {
public:
    GLuint textureID;
    int width, height, channels;
    
    Texture() : textureID(0), width(0), height(0), channels(0) {}
    ~Texture();
    
    bool LoadFromFile(const std::string& filepath);
    bool CreateFromData(unsigned char* data, int w, int h, int ch);
    bool CreateSolid(int w, int h, const glm::vec4& color);
    void Bind(int unit = 0) const;
};

// ============================================================================
// Star (for background)
// ============================================================================

struct Star {
    glm::vec2 position;
    float size;
    float brightness;
    float twinkleSpeed;
    float twinklePhase;
};

// ============================================================================
// Font Character (for text rendering)
// ============================================================================

struct Character {
    GLuint textureID;   // ID handle of the glyph texture
    glm::ivec2 size;    // Size of glyph
    glm::ivec2 bearing; // Offset from baseline to left/top of glyph
    unsigned int advance;    // Offset to advance to next glyph
};

// ============================================================================
// Renderer Class
// ============================================================================

class Renderer {
public:
    Renderer();
    ~Renderer();
    
    bool Initialize(int windowWidth, int windowHeight);
    void Shutdown();
    
    // Frame management
    void BeginFrame();
    void EndFrame();
    
    // Camera
    void SetCameraPosition(const glm::vec2& position);
    glm::vec2 GetCameraPosition() const { return cameraPosition; }
    glm::vec2 ScreenToWorld(const glm::vec2& screenPos) const;
    glm::vec2 WorldToScreen(const glm::vec2& worldPos) const;
    
    // Sprite rendering
    void DrawSprite(const glm::vec2& position, const glm::vec2& size, float rotation,
                   const glm::vec4& color, Texture* texture = nullptr);
    void DrawSpriteWorld(const glm::vec2& position, const glm::vec2& size, float rotation,
                        const glm::vec4& color, Texture* texture = nullptr);
    void DrawSpriteWorld(const glm::vec2& position, const glm::vec2& size, float rotation,
                        const glm::vec4& color, Texture* texture, bool flipHorizontal);
    
    // Shape rendering (using sprite quad with white texture)
    void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
    void DrawQuadWorld(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
    void DrawCircle(const glm::vec2& position, float radius, const glm::vec4& color, int segments = 16);
    void DrawCircleWorld(const glm::vec2& position, float radius, const glm::vec4& color, int segments = 16);
    void DrawLine(const glm::vec2& start, const glm::vec2& end, float thickness, const glm::vec4& color);
    void DrawLineWorld(const glm::vec2& start, const glm::vec2& end, float thickness, const glm::vec4& color);
    
    // UI rendering (screen space)
    void DrawRect(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
    void DrawRectOutline(const glm::vec2& position, const glm::vec2& size, float thickness, const glm::vec4& color);
    void DrawProgressBar(const glm::vec2& position, const glm::vec2& size, float progress,
                        const glm::vec4& fillColor, const glm::vec4& bgColor);
    void DrawText(const std::string& text, const glm::vec2& position, float scale, const glm::vec4& color);
    
    // Background
    void DrawStarfield(float time);
    void InitStarfield(int starCount);
    
    // Particles (batch rendered)
    void DrawParticle(const glm::vec2& position, float size, const glm::vec4& color);
    void FlushParticles();
    
    // Screen effects
    void SetScreenShake(float intensity, float duration);
    void UpdateScreenShake(float deltaTime);
    
    // Getters
    int GetWindowWidth() const { return windowWidth; }
    int GetWindowHeight() const { return windowHeight; }
    Texture* GetWhiteTexture() { return &whiteTexture; }
    
private:
    void InitQuadBuffers();
    void InitParticleBuffers();
    void InitTextRendering();
    glm::mat4 GetProjectionMatrix() const;
    glm::mat4 GetViewMatrix() const;
    
    // Window dimensions
    int windowWidth, windowHeight;
    
    // Camera
    glm::vec2 cameraPosition;
    glm::vec2 cameraShakeOffset;
    float shakeIntensity;
    float shakeDuration;
    float shakeTimer;
    
    // Shaders
    Shader spriteShader;
    Shader particleShader;
    Shader textShader;
    
    // Buffers for sprite rendering
    GLuint quadVAO, quadVBO, quadEBO;
    
    // Buffers for particle batching
    GLuint particleVAO, particleVBO;
    std::vector<Vertex> particleVertices;
    static const int MAX_PARTICLES_PER_BATCH = 10000;
    
    // Buffers for text rendering
    GLuint textVAO, textVBO;
    
    // Default textures
    Texture whiteTexture;
    
    // Starfield
    std::vector<Star> stars;
    
    // Font rendering (FreeType)
    FT_Library ftLibrary;
    FT_Face ftFace;
    std::map<char, Character> characters;
    bool fontInitialized;
};

// ============================================================================
// Global Renderer Access (set by Game)
// ============================================================================

extern Renderer* g_Renderer;

#endif // RENDERER_H
