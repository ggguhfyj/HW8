#version 300 es

/**
 * \file
 * \author JUNSEOK LEE
 * \date 2025 Fall
 * \par CS200 Computer Graphics I
 * \copyright DigiPen Institute of Technology
 */

layout(std140) uniform Camera
{
    mat3 uViewProjection;
};

layout(location = 0) in vec2 aPosition; // this is a value that spans -0.5 to 0.5 (open gl standard for model positions)

uniform mat3 uModel;

uniform vec2 quadsize;

out vec2 testpoint;

void main()
{
    //local -> world -> camera -> clip-> screen 

    testpoint = quadsize * aPosition;


    vec3 world_pos = uModel * vec3(aPosition, 1.0); // placing the model in the world

    vec3 ndc_pos = uViewProjection *world_pos; // translating the model by the view projection aka camera. range is -1 to 1.
    //-1 to 1

    gl_Position = vec4(ndc_pos.xy, 0.0, 1.0); // When a vertex shader outputs gl_Position, it’s in clip space. discard all that dont fit in -1 to 1.
    // expected range for visible vertices is -1 to 1;

}
