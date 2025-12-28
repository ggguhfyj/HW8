#version 300 es
precision         mediump float;
precision mediump sampler2D;

/**
 * \file
 * \author JUNSEOK
 * \date 2025 Fall
 * \par CS200 Computer Graphics I
 * \copyright DigiPen Institute of Technology
 */


in vec2 vTexCoord;

out vec4 FragColor;

uniform sampler2D uTex2d;
uniform vec4 uTintColor;

void main()
{
    vec4 texColor = texture(uTex2d, vTexCoord);
    vec4 finalColor = texColor * uTintColor;

    if(finalColor.a == 0.0)
    {
        discard; // discard the color so that we may see through it
    }

    FragColor = finalColor;
}