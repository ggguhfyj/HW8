#version 300 es

layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec4 aColor;

uniform mat3 uMVP;
uniform float uPointSize;

out vec4 vColor;

void main()
{
    vec3 clip = uMVP * vec3(aPosition, 1.0);
    gl_Position = vec4(clip.xy, 0.0, 1.0);
    gl_PointSize = uPointSize;
    vColor       = aColor;
}
