#version 330 core

// ============================================================================
// Galaxies Away - Particle Fragment Shader
// Creates circular particles with soft edges
// ============================================================================

in vec4 Color;

out vec4 FragColor;

void main() {
    // Create circular particle shape
    vec2 coord = gl_PointCoord - vec2(0.5);
    float dist = length(coord);
    if (dist > 0.5) {
        discard;
    }
    
    // Apply soft edge for smoother appearance
    float alpha = 1.0 - smoothstep(0.3, 0.5, dist);
    FragColor = Color * vec4(1.0, 1.0, 1.0, alpha);
}
