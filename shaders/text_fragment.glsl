#version 330 core

// ============================================================================
// Galaxies Away - Text Fragment Shader
// Renders FreeType font glyphs with color
// ============================================================================

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D text;
uniform vec4 textColor;

void main() {
    // Sample the red channel (grayscale glyph texture)
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
    FragColor = textColor * sampled;
}
