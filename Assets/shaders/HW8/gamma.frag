#version 300 es

// Author: JUNSEOK LEE
// Date: 2025 December 29
// as the days dwindle down to a precious few

precision mediump float;

in vec2 vTexCoord;

uniform sampler2D uInput;
uniform float uGamma;

out vec4 FragColor;

void main()
{
    vec3 color = texture(uInput, vTexCoord).rgb;
    float inv_gamma = 1.0 / max(uGamma, 0.001);
    color = pow(color, vec3(inv_gamma));
    FragColor = vec4(color, 1.0);
}
