#version 330 core

// ============================================================================
// Galaxies Away - Particle Vertex Shader
// Transforms particle positions for batch rendering
// ============================================================================

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec4 aColor;

out vec4 Color;

uniform mat4 projection;
uniform mat4 view;

void main() {
    gl_Position = projection * view * vec4(aPos, 0.0, 1.0);
    Color = aColor;
}
