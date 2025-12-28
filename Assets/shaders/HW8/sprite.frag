#version 300 es

// Author: JUNSEOK LEE
// Date: 2025 December 29
// as the days dwindle down to a precious few

precision mediump float;

in vec2 vTexCoord;

uniform sampler2D uTexture;
uniform vec4 uTintColor;

out vec4 FragColor;

void main()
{
    vec4 tex = texture(uTexture, vTexCoord);
    FragColor = tex * uTintColor;
}
