#version 330 core

// ============================================================================
// Galaxies Away - Vertex Shader
// Handles sprite transformation and passes data to fragment shader
// ============================================================================

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec4 aColor;

out vec2 TexCoord;
out vec4 Color;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main() {
    // Transform vertex position
    gl_Position = projection * view * model * vec4(aPos, 0.0, 1.0);
    
    // Pass through texture coordinates and color
    TexCoord = aTexCoord;
    Color = aColor;
}
