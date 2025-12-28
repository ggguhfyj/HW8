#version 300 es

precision mediump float;

in vec2 vTexCoord;

uniform sampler2D uInput;
uniform float uIntensity;
uniform float uRadius;
uniform float uSoftness;

out vec4 FragColor;

void main()
{
    vec3 color = texture(uInput, vTexCoord).rgb;
    float dist = distance(vTexCoord, vec2(0.5));
    float vignette = smoothstep(uRadius, uRadius - uSoftness, dist);
    float factor = mix(1.0, vignette, uIntensity);
    color *= factor;
    FragColor = vec4(color, 1.0);
}
