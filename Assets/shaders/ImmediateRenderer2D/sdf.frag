#version 300 es
precision mediump float;

/**
 * \file
 * \author Junseok Lee
 * \date 2025 Fall
 * \par CS200 Computer Graphics I
 * \copyright DigiPen Institute of Technology
 */

out vec4 FragColor;

uniform int shapetype; // iguess we need to pass a variable of this name in a later stage... 

uniform vec2 size; // was u world size

uniform vec4 outline_color;

uniform vec4 fill_color;

in vec2 testpoint;

uniform float uLineWidth;



float sdCircle( vec2 p, float r )
{
    return length(p) - r;
}

float sdRectangle(vec2 point, vec2 half_dim)
{
    vec2 distance_to_edges = abs(point) - half_dim; // take everything to the first quad

    float outside_distance = length(max(distance_to_edges, 0.0));

    float inside_distance = min(max(distance_to_edges.x, distance_to_edges.y), 0.0);

    float sdf = outside_distance + inside_distance;

    return sdf;
}

// evalute the color based off sdf
vec4 evaluate_color(float sdf)
{
    float fill_alpha = (sdf < 0.0) ? 1.0 : 0.0;
    float outline_alpha = (abs(sdf) < 0.5 *uLineWidth) ? 1.0 : 0.0;

    vec4 base_fill = vec4(fill_color.rgb, fill_alpha * fill_color.a);
    vec4 base_outline = vec4(outline_color.rgb, outline_alpha * outline_color.a);

    return mix(base_fill, base_outline, base_outline.a);
}

void main()
{
 // based off shape evaluate the sdf
    float sdf = 0.0;
    if(shapetype == 0)
    {
        float radius = min(size.x, size.y)*0.5;
        sdf = sdCircle(testpoint, radius);
    }else if(shapetype == 1)
    {
        sdf = sdRectangle(testpoint, 0.5*size);
    }
 // get the color
    vec4 color = evaluate_color(sdf);
    if(color.a <= 0.0)
        discard;
 // set color, discard empty space
    FragColor = color;
}
