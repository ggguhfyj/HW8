#version 300 es

precision mediump float;

in vec2 vTexCoord;

uniform sampler2D uInput;
uniform vec2 uTexelSize;
uniform float uStrength;

out vec4 FragColor;

void main()
{
    vec2 centered = vTexCoord - vec2(0.5);
    vec2 offset = centered * uStrength * uTexelSize;

    float r = texture(uInput, vTexCoord + offset).r;
    float g = texture(uInput, vTexCoord).g;
    float b = texture(uInput, vTexCoord - offset).b;

    FragColor = vec4(r, g, b, 1.0);
}
