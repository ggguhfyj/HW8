#version 300 es

// Author: JUNSEOK LEE
// Date: 2025 December 29
// as the days dwindle down to a precious few

precision highp float;

out vec2 vTexCoord;

void main()
{
    const vec2 positions[3] = vec2[3](
        vec2(-1.0, -1.0),
        vec2(3.0, -1.0),
        vec2(-1.0, 3.0)
    );

    vec2 pos = positions[gl_VertexID];
    vTexCoord = pos * 0.5 + 0.5;
    gl_Position = vec4(pos, 0.0, 1.0);
}
