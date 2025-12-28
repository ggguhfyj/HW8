#version 300 es

precision mediump float;

in vec2 vTexCoord;

uniform sampler2D uInput;
uniform vec2 uResolution;
uniform float uTime;
uniform float uGrainIntensity;
uniform float uScanlineIntensity;

out vec4 FragColor;

float rand(vec2 co)
{
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
    vec3 color = texture(uInput, vTexCoord).rgb;
    vec2 uv = vTexCoord * uResolution;
    float noise = rand(uv + uTime * 60.0);
    float grain = (noise - 0.5) * uGrainIntensity;
    color += grain;

    float scan = sin(vTexCoord.y * uResolution.y * 3.14159);
    float scan_factor = 1.0 - uScanlineIntensity * (0.5 - 0.5 * scan);
    color *= scan_factor;

    FragColor = vec4(color, 1.0);
}
