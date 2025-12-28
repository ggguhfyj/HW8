#version 300 es

// Author: JUNSEOK LEE
// Date: 2025 December 29
// as the days dwindle down to a precious few

precision highp float;

layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aTexCoord;

uniform mat3 uViewProjection;
uniform mat3 uModel;
uniform float uDepth;

out vec2 vTexCoord;

void main()
{
    vec3 world_pos = uModel * vec3(aPosition, 1.0);
    vec3 ndc_pos = uViewProjection * world_pos;
    float depth = clamp(uDepth, 0.0, 1.0);
    gl_Position = vec4(ndc_pos.xy, depth * 2.0 - 1.0, 1.0);
    vTexCoord = aTexCoord;
}
