#version 330 core

// ============================================================================
// Galaxies Away - Fragment Shader
// Handles texture sampling and color blending
// ============================================================================

in vec2 TexCoord;
in vec4 Color;

out vec4 FragColor;

uniform sampler2D textureSampler;
uniform bool useTexture;

void main() {
    // Sample texture if enabled, otherwise use white
    vec4 texColor = useTexture ? texture(textureSampler, TexCoord) : vec4(1.0);
    
    // Multiply by vertex color
    FragColor = texColor * Color;
    
    // Discard fully transparent pixels for better blending
    if (FragColor.a < 0.01) {
        discard;
    }
}
