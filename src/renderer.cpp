// ============================================================================
// Galaxies Away - OpenGL Renderer Implementation
// ============================================================================

#include "renderer.h"
#include <cstring>

// Global renderer pointer
Renderer* g_Renderer = nullptr;

// ============================================================================
// Shader Implementation
// ============================================================================

Shader::~Shader() {
    if (programID) {
        glDeleteProgram(programID);
    }
}

bool Shader::LoadFromStrings(const std::string& vertexSource, const std::string& fragmentSource) {
    GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexSource);
    if (!vertexShader) return false;
    
    GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);
    if (!fragmentShader) {
        glDeleteShader(vertexShader);
        return false;
    }
    
    programID = glCreateProgram();
    glAttachShader(programID, vertexShader);
    glAttachShader(programID, fragmentShader);
    glLinkProgram(programID);
    
    // Check for linking errors
    GLint success;
    glGetProgramiv(programID, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(programID, 512, nullptr, infoLog);
        std::cerr << "Shader program linking failed:\n" << infoLog << std::endl;
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return false;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return true;
}

bool Shader::LoadFromFiles(const std::string& vertexPath, const std::string& fragmentPath) {
    std::string vertexSource = Utils::ReadFile(vertexPath);
    std::string fragmentSource = Utils::ReadFile(fragmentPath);
    
    if (vertexSource.empty() || fragmentSource.empty()) {
        return false;
    }
    
    return LoadFromStrings(vertexSource, fragmentSource);
}

void Shader::Use() const {
    glUseProgram(programID);
}

void Shader::SetInt(const std::string& name, int value) const {
    glUniform1i(glGetUniformLocation(programID, name.c_str()), value);
}

void Shader::SetFloat(const std::string& name, float value) const {
    glUniform1f(glGetUniformLocation(programID, name.c_str()), value);
}

void Shader::SetVec2(const std::string& name, const glm::vec2& value) const {
    glUniform2f(glGetUniformLocation(programID, name.c_str()), value.x, value.y);
}

void Shader::SetVec3(const std::string& name, const glm::vec3& value) const {
    glUniform3f(glGetUniformLocation(programID, name.c_str()), value.x, value.y, value.z);
}

void Shader::SetVec4(const std::string& name, const glm::vec4& value) const {
    glUniform4f(glGetUniformLocation(programID, name.c_str()), value.x, value.y, value.z, value.w);
}

void Shader::SetMat4(const std::string& name, const glm::mat4& value) const {
    glUniformMatrix4fv(glGetUniformLocation(programID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

GLuint Shader::CompileShader(GLenum type, const std::string& source) {
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    
    if (!CheckCompileErrors(shader, type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT")) {
        glDeleteShader(shader);
        return 0;
    }
    
    return shader;
}

bool Shader::CheckCompileErrors(GLuint shader, const std::string& type) {
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << type << " shader compilation failed:\n" << infoLog << std::endl;
        return false;
    }
    return true;
}

// ============================================================================
// Texture Implementation
// ============================================================================

Texture::~Texture() {
    if (textureID) {
        glDeleteTextures(1, &textureID);
    }
}

bool Texture::LoadFromFile(const std::string& filepath) {
    // For simplicity, we'll create a placeholder texture
    // In a full implementation, you would use stb_image here
    std::cerr << "Note: Texture loading from file not implemented, using placeholder for: " 
              << filepath << std::endl;
    return CreateSolid(64, 64, Colors::WHITE);
}

bool Texture::CreateFromData(unsigned char* data, int w, int h, int ch) {
    width = w;
    height = h;
    channels = ch;
    
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    return true;
}

bool Texture::CreateSolid(int w, int h, const glm::vec4& color) {
    std::vector<unsigned char> data(w * h * 4);
    for (int i = 0; i < w * h; ++i) {
        data[i * 4 + 0] = static_cast<unsigned char>(color.r * 255);
        data[i * 4 + 1] = static_cast<unsigned char>(color.g * 255);
        data[i * 4 + 2] = static_cast<unsigned char>(color.b * 255);
        data[i * 4 + 3] = static_cast<unsigned char>(color.a * 255);
    }
    return CreateFromData(data.data(), w, h, 4);
}

void Texture::Bind(int unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, textureID);
}

// ============================================================================
// Renderer Implementation
// ============================================================================

Renderer::Renderer()
    : windowWidth(0), windowHeight(0)
    , cameraPosition(0.0f)
    , cameraShakeOffset(0.0f)
    , shakeIntensity(0.0f)
    , shakeDuration(0.0f)
    , shakeTimer(0.0f)
    , quadVAO(0), quadVBO(0), quadEBO(0)
    , particleVAO(0), particleVBO(0)
    , textVAO(0), textVBO(0)
    , ftLibrary(nullptr)
    , ftFace(nullptr)
    , fontInitialized(false)
{
}

Renderer::~Renderer() {
    Shutdown();
}

bool Renderer::Initialize(int width, int height) {
    windowWidth = width;
    windowHeight = height;
    
    // Print OpenGL info
    std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    
    // Set up OpenGL state
    glViewport(0, 0, windowWidth, windowHeight);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    
    // Load shaders from files
    if (!spriteShader.LoadFromFiles("shaders/vertex.glsl", "shaders/fragment.glsl")) {
        std::cerr << "Failed to load sprite shader" << std::endl;
        return false;
    }
    
    if (!particleShader.LoadFromFiles("shaders/particle_vertex.glsl", "shaders/particle_fragment.glsl")) {
        std::cerr << "Failed to load particle shader" << std::endl;
        return false;
    }
    
    if (!textShader.LoadFromFiles("shaders/text_vertex.glsl", "shaders/text_fragment.glsl")) {
        std::cerr << "Failed to load text shader" << std::endl;
        return false;
    }
    
    // Initialize buffers
    InitQuadBuffers();
    InitParticleBuffers();
    
    // Create white texture for solid color rendering
    whiteTexture.CreateSolid(4, 4, Colors::WHITE);
    
    // Initialize starfield
    InitStarfield(300);
    
    // Initialize text rendering
    InitTextRendering();
    
    return true;
}

void Renderer::Shutdown() {
    // Clean up text rendering resources
    if (textVAO) {
        glDeleteVertexArrays(1, &textVAO);
        textVAO = 0;
    }
    if (textVBO) {
        glDeleteBuffers(1, &textVBO);
        textVBO = 0;
    }
    
    // Clean up font character textures
    for (auto& pair : characters) {
        glDeleteTextures(1, &pair.second.textureID);
    }
    characters.clear();
    
    // Clean up FreeType resources
    if (ftFace) {
        FT_Done_Face(ftFace);
        ftFace = nullptr;
    }
    if (ftLibrary) {
        FT_Done_FreeType(ftLibrary);
        ftLibrary = nullptr;
    }
    
    if (quadVAO) {
        glDeleteVertexArrays(1, &quadVAO);
        quadVAO = 0;
    }
    if (quadVBO) {
        glDeleteBuffers(1, &quadVBO);
        quadVBO = 0;
    }
    if (quadEBO) {
        glDeleteBuffers(1, &quadEBO);
        quadEBO = 0;
    }
    if (particleVAO) {
        glDeleteVertexArrays(1, &particleVAO);
        particleVAO = 0;
    }
    if (particleVBO) {
        glDeleteBuffers(1, &particleVBO);
        particleVBO = 0;
    }
}

void Renderer::InitQuadBuffers() {
    // Quad vertices: position (2), texcoord (2), color (4)
    float vertices[] = {
        // Position      // TexCoord  // Color (white default)
        -0.5f, -0.5f,    0.0f, 0.0f,  1.0f, 1.0f, 1.0f, 1.0f,
         0.5f, -0.5f,    1.0f, 0.0f,  1.0f, 1.0f, 1.0f, 1.0f,
         0.5f,  0.5f,    1.0f, 1.0f,  1.0f, 1.0f, 1.0f, 1.0f,
        -0.5f,  0.5f,    0.0f, 1.0f,  1.0f, 1.0f, 1.0f, 1.0f
    };
    
    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };
    
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glGenBuffers(1, &quadEBO);
    
    glBindVertexArray(quadVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // TexCoord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Color attribute
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(4 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    glBindVertexArray(0);
}

void Renderer::InitParticleBuffers() {
    particleVertices.reserve(MAX_PARTICLES_PER_BATCH * 6); // 6 vertices per particle (2 triangles)
    
    glGenVertexArrays(1, &particleVAO);
    glGenBuffers(1, &particleVBO);
    
    glBindVertexArray(particleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
    glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES_PER_BATCH * 6 * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);
    
    // Position
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);
    
    // TexCoord
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
    glEnableVertexAttribArray(1);
    
    // Color
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(2);
    
    glBindVertexArray(0);
}

void Renderer::InitTextRendering() {
    // Initialize FreeType library
    if (FT_Init_FreeType(&ftLibrary)) {
        std::cerr << "Could not initialize FreeType library" << std::endl;
        fontInitialized = false;
        return;
    }
    
    // Try to load a default system font
    // Common paths for different operating systems
    std::vector<std::string> fontPaths = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",           // Linux
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf", // Linux
        "/System/Library/Fonts/Helvetica.ttc",                        // macOS
        "/Library/Fonts/Arial.ttf",                                   // macOS
        "C:/Windows/Fonts/arial.ttf"                                  // Windows
    };
    
    bool fontLoaded = false;
    for (const auto& path : fontPaths) {
        if (FT_New_Face(ftLibrary, path.c_str(), 0, &ftFace) == 0) {
            std::cout << "Loaded font: " << path << std::endl;
            fontLoaded = true;
            break;
        }
    }
    
    if (!fontLoaded) {
        std::cerr << "Failed to load any system font. Text rendering will not work." << std::endl;
        FT_Done_FreeType(ftLibrary);
        ftLibrary = nullptr;
        fontInitialized = false;
        return;
    }
    
    // Set font size (48 pixels height)
    FT_Set_Pixel_Sizes(ftFace, 0, 48);
    
    // Disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    // Load first 128 ASCII characters
    for (unsigned char c = 0; c < 128; c++) {
        // Load character glyph
        if (FT_Load_Char(ftFace, c, FT_LOAD_RENDER)) {
            std::cerr << "Failed to load glyph for character: " << c << std::endl;
            continue;
        }
        
        // Generate texture
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            ftFace->glyph->bitmap.width,
            ftFace->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            ftFace->glyph->bitmap.buffer
        );
        
        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        // Store character for later use
        Character character = {
            texture,
            glm::ivec2(ftFace->glyph->bitmap.width, ftFace->glyph->bitmap.rows),
            glm::ivec2(ftFace->glyph->bitmap_left, ftFace->glyph->bitmap_top),
            static_cast<unsigned int>(ftFace->glyph->advance.x)
        };
        characters.insert(std::pair<char, Character>(c, character));
    }
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    // Configure VAO/VBO for texture quads
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    fontInitialized = true;
    std::cout << "Text rendering initialized successfully" << std::endl;
}

void Renderer::InitStarfield(int starCount) {
    stars.clear();
    stars.reserve(starCount);
    
    for (int i = 0; i < starCount; ++i) {
        Star star;
        star.position = glm::vec2(
            Utils::RandomFloat(-Constants::WORLD_WIDTH, Constants::WORLD_WIDTH * 2),
            Utils::RandomFloat(-Constants::WORLD_HEIGHT, Constants::WORLD_HEIGHT * 2)
        );
        star.size = Utils::RandomFloat(1.0f, 3.0f);
        star.brightness = Utils::RandomFloat(0.3f, 1.0f);
        star.twinkleSpeed = Utils::RandomFloat(1.0f, 3.0f);
        star.twinklePhase = Utils::RandomFloat(0.0f, glm::two_pi<float>());
        stars.push_back(star);
    }
}

void Renderer::BeginFrame() {
    glClearColor(0.02f, 0.02f, 0.08f, 1.0f); // Dark space blue
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::EndFrame() {
    // Flush any remaining particles
    FlushParticles();
}

void Renderer::SetCameraPosition(const glm::vec2& position) {
    cameraPosition = position;
}

glm::vec2 Renderer::ScreenToWorld(const glm::vec2& screenPos) const {
    glm::vec2 centered = screenPos - glm::vec2(windowWidth * 0.5f, windowHeight * 0.5f);
    return centered + cameraPosition;
}

glm::vec2 Renderer::WorldToScreen(const glm::vec2& worldPos) const {
    glm::vec2 relative = worldPos - cameraPosition;
    return relative + glm::vec2(windowWidth * 0.5f, windowHeight * 0.5f);
}

glm::mat4 Renderer::GetProjectionMatrix() const {
    return glm::ortho(0.0f, (float)windowWidth, (float)windowHeight, 0.0f, -1.0f, 1.0f);
}

glm::mat4 Renderer::GetViewMatrix() const {
    glm::mat4 view = glm::mat4(1.0f);
    view = glm::translate(view, glm::vec3(windowWidth * 0.5f, windowHeight * 0.5f, 0.0f));
    view = glm::translate(view, glm::vec3(-cameraPosition.x + cameraShakeOffset.x, 
                                          -cameraPosition.y + cameraShakeOffset.y, 0.0f));
    return view;
}

void Renderer::DrawSprite(const glm::vec2& position, const glm::vec2& size, float rotation,
                         const glm::vec4& color, Texture* texture) {
    spriteShader.Use();
    
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(position, 0.0f));
    model = glm::rotate(model, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(size, 1.0f));
    
    glm::mat4 projection = GetProjectionMatrix();
    glm::mat4 view = glm::mat4(1.0f); // Screen space
    
    spriteShader.SetMat4("projection", projection);
    spriteShader.SetMat4("view", view);
    spriteShader.SetMat4("model", model);
    spriteShader.SetVec4("color", color);
    
    if (texture) {
        texture->Bind(0);
        spriteShader.SetInt("textureSampler", 0);
        spriteShader.SetInt("useTexture", 1);
    } else {
        whiteTexture.Bind(0);
        spriteShader.SetInt("textureSampler", 0);
        spriteShader.SetInt("useTexture", 0);
    }
    
    // Update vertex colors
    float vertices[] = {
        -0.5f, -0.5f,  0.0f, 0.0f,  color.r, color.g, color.b, color.a,
         0.5f, -0.5f,  1.0f, 0.0f,  color.r, color.g, color.b, color.a,
         0.5f,  0.5f,  1.0f, 1.0f,  color.r, color.g, color.b, color.a,
        -0.5f,  0.5f,  0.0f, 1.0f,  color.r, color.g, color.b, color.a
    };
    
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    
    glBindVertexArray(quadVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Renderer::DrawSpriteWorld(const glm::vec2& position, const glm::vec2& size, float rotation,
                              const glm::vec4& color, Texture* texture) {
    spriteShader.Use();
    
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(position, 0.0f));
    model = glm::rotate(model, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(size, 1.0f));
    
    glm::mat4 projection = GetProjectionMatrix();
    glm::mat4 view = GetViewMatrix();
    
    spriteShader.SetMat4("projection", projection);
    spriteShader.SetMat4("view", view);
    spriteShader.SetMat4("model", model);
    
    if (texture) {
        texture->Bind(0);
        spriteShader.SetInt("textureSampler", 0);
        spriteShader.SetInt("useTexture", 1);
    } else {
        whiteTexture.Bind(0);
        spriteShader.SetInt("textureSampler", 0);
        spriteShader.SetInt("useTexture", 0);
    }
    
    // Update vertex colors
    float vertices[] = {
        -0.5f, -0.5f,  0.0f, 0.0f,  color.r, color.g, color.b, color.a,
         0.5f, -0.5f,  1.0f, 0.0f,  color.r, color.g, color.b, color.a,
         0.5f,  0.5f,  1.0f, 1.0f,  color.r, color.g, color.b, color.a,
        -0.5f,  0.5f,  0.0f, 1.0f,  color.r, color.g, color.b, color.a
    };
    
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    
    glBindVertexArray(quadVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Renderer::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color) {
    DrawSprite(position, size, 0.0f, color, nullptr);
}

void Renderer::DrawQuadWorld(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color) {
    DrawSpriteWorld(position, size, 0.0f, color, nullptr);
}

void Renderer::DrawCircle(const glm::vec2& position, float radius, const glm::vec4& color, int segments) {
    // Draw circle as a series of triangles
    // For simplicity, we'll just draw a quad for now (would be circular with proper texture/shader)
    DrawQuad(position, glm::vec2(radius * 2), color);
}

void Renderer::DrawCircleWorld(const glm::vec2& position, float radius, const glm::vec4& color, int segments) {
    DrawQuadWorld(position, glm::vec2(radius * 2), color);
}

void Renderer::DrawLine(const glm::vec2& start, const glm::vec2& end, float thickness, const glm::vec4& color) {
    glm::vec2 direction = end - start;
    float length = Utils::Length(direction);
    float angle = Utils::Angle(direction);
    glm::vec2 center = (start + end) * 0.5f;
    
    DrawSprite(center, glm::vec2(length, thickness), angle, color, nullptr);
}

void Renderer::DrawLineWorld(const glm::vec2& start, const glm::vec2& end, float thickness, const glm::vec4& color) {
    glm::vec2 direction = end - start;
    float length = Utils::Length(direction);
    float angle = Utils::Angle(direction);
    glm::vec2 center = (start + end) * 0.5f;
    
    DrawSpriteWorld(center, glm::vec2(length, thickness), angle, color, nullptr);
}

void Renderer::DrawRect(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color) {
    // Position is top-left corner
    glm::vec2 center = position + size * 0.5f;
    DrawSprite(center, size, 0.0f, color, nullptr);
}

void Renderer::DrawRectOutline(const glm::vec2& position, const glm::vec2& size, float thickness, const glm::vec4& color) {
    // Top
    DrawRect(position, glm::vec2(size.x, thickness), color);
    // Bottom
    DrawRect(glm::vec2(position.x, position.y + size.y - thickness), glm::vec2(size.x, thickness), color);
    // Left
    DrawRect(position, glm::vec2(thickness, size.y), color);
    // Right
    DrawRect(glm::vec2(position.x + size.x - thickness, position.y), glm::vec2(thickness, size.y), color);
}

void Renderer::DrawProgressBar(const glm::vec2& position, const glm::vec2& size, float progress,
                              const glm::vec4& fillColor, const glm::vec4& bgColor) {
    progress = Utils::Clamp(progress, 0.0f, 1.0f);
    
    // Background
    DrawRect(position, size, bgColor);
    
    // Fill
    if (progress > 0.0f) {
        float padding = 2.0f;
        glm::vec2 fillPos = position + glm::vec2(padding);
        glm::vec2 fillSize = glm::vec2((size.x - padding * 2) * progress, size.y - padding * 2);
        DrawRect(fillPos, fillSize, fillColor);
    }
}

void Renderer::DrawText(const std::string& text, const glm::vec2& position, float scale, const glm::vec4& color) {
    if (!fontInitialized || characters.empty()) {
        // Fallback to simple rectangle rendering if font not initialized
        float charWidth = 10.0f * scale;
        float charHeight = 16.0f * scale;
        float spacing = 2.0f * scale;
        glm::vec2 cursor = position;
        
        for (char c : text) {
            if (c == ' ') {
                cursor.x += charWidth + spacing;
                continue;
            }
            if (c == '\n') {
                cursor.x = position.x;
                cursor.y += charHeight + spacing;
                continue;
            }
            DrawRect(cursor, glm::vec2(charWidth, charHeight), color);
            cursor.x += charWidth + spacing;
        }
        return;
    }
    
    // Use text shader for proper font rendering
    textShader.Use();
    textShader.SetVec4("textColor", color);
    textShader.SetMat4("projection", GetProjectionMatrix());
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(textVAO);
    
    // Iterate through all characters
    float x = position.x;
    float y = position.y;
    
    for (auto it = text.begin(); it != text.end(); ++it) {
        char c = *it;
        
        // Handle newlines
        if (c == '\n') {
            x = position.x;
            y -= 48.0f * scale; // Move down one line (font size is 48)
            continue;
        }
        
        // Get character data
        if (characters.find(c) == characters.end()) {
            continue; // Skip characters we don't have
        }
        
        Character ch = characters[c];
        
        // Calculate position
        // In a top-down coordinate system (0 at top), we need to add bearing for correct baseline
        float xpos = x + ch.bearing.x * scale;
        float ypos = y + (ch.size.y - ch.bearing.y) * scale;
        
        float w = ch.size.x * scale;
        float h = ch.size.y * scale;
        
        // Update VBO for each character
        // Flip texture coordinates vertically (FreeType uses top-down, OpenGL uses bottom-up)
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 1.0f },
            { xpos,     ypos,       0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 0.0f },
            
            { xpos,     ypos + h,   0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 0.0f },
            { xpos + w, ypos + h,   1.0f, 1.0f }
        };
        
        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.textureID);
        
        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        // Advance cursor for next glyph
        // Bitshift by 6 to get value in pixels (2^6 = 64 - FreeType uses 1/64th pixels)
        x += (ch.advance >> 6) * scale;
    }
    
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::DrawStarfield(float time) {
    for (const Star& star : stars) {
        // Calculate twinkle effect
        float twinkle = 0.5f + 0.5f * std::sin(time * star.twinkleSpeed + star.twinklePhase);
        float alpha = star.brightness * (0.5f + 0.5f * twinkle);
        
        glm::vec4 starColor(1.0f, 1.0f, 1.0f, alpha);
        
        // Draw star in world space
        DrawSpriteWorld(star.position, glm::vec2(star.size), 0.0f, starColor, nullptr);
    }
}

void Renderer::DrawParticle(const glm::vec2& position, float size, const glm::vec4& color) {
    // Add particle vertices (6 vertices for a quad - 2 triangles)
    float halfSize = size * 0.5f;
    
    Vertex v1(glm::vec2(position.x - halfSize, position.y - halfSize), glm::vec2(0, 0), color);
    Vertex v2(glm::vec2(position.x + halfSize, position.y - halfSize), glm::vec2(1, 0), color);
    Vertex v3(glm::vec2(position.x + halfSize, position.y + halfSize), glm::vec2(1, 1), color);
    Vertex v4(glm::vec2(position.x - halfSize, position.y - halfSize), glm::vec2(0, 0), color);
    Vertex v5(glm::vec2(position.x + halfSize, position.y + halfSize), glm::vec2(1, 1), color);
    Vertex v6(glm::vec2(position.x - halfSize, position.y + halfSize), glm::vec2(0, 1), color);
    
    particleVertices.push_back(v1);
    particleVertices.push_back(v2);
    particleVertices.push_back(v3);
    particleVertices.push_back(v4);
    particleVertices.push_back(v5);
    particleVertices.push_back(v6);
    
    // Flush if buffer is full
    if (particleVertices.size() >= MAX_PARTICLES_PER_BATCH * 6) {
        FlushParticles();
    }
}

void Renderer::FlushParticles() {
    if (particleVertices.empty()) return;
    
    spriteShader.Use();
    spriteShader.SetMat4("projection", GetProjectionMatrix());
    spriteShader.SetMat4("view", GetViewMatrix());
    spriteShader.SetMat4("model", glm::mat4(1.0f));
    spriteShader.SetInt("useTexture", 0);
    
    whiteTexture.Bind(0);
    spriteShader.SetInt("textureSampler", 0);
    
    glBindVertexArray(particleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, particleVertices.size() * sizeof(Vertex), particleVertices.data());
    
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(particleVertices.size()));
    
    glBindVertexArray(0);
    particleVertices.clear();
}

void Renderer::SetScreenShake(float intensity, float duration) {
    shakeIntensity = intensity;
    shakeDuration = duration;
    shakeTimer = 0.0f;
}

void Renderer::UpdateScreenShake(float deltaTime) {
    if (shakeDuration <= 0.0f) {
        cameraShakeOffset = glm::vec2(0.0f);
        return;
    }
    
    shakeTimer += deltaTime;
    if (shakeTimer >= shakeDuration) {
        shakeDuration = 0.0f;
        cameraShakeOffset = glm::vec2(0.0f);
        return;
    }
    
    // Calculate shake with decay
    float decay = 1.0f - (shakeTimer / shakeDuration);
    float currentIntensity = shakeIntensity * decay;
    
    cameraShakeOffset = glm::vec2(
        Utils::RandomFloat(-currentIntensity, currentIntensity),
        Utils::RandomFloat(-currentIntensity, currentIntensity)
    );
}
