#version 300 es

/**
 * \file
 * \author JUNSEOK
 * \date 2025 Fall
 * \par CS200 Computer Graphics I
 * \copyright DigiPen Institute of Technology 

 * the vertex shader is first
 */

layout(std140) uniform Camera
{
    mat3 uViewProjection;
};

layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aTexCoord;

out vec2 vTexCoord; // to pass to the fragment shader

uniform mat3 uModel;
uniform mat3 uTextureTransform;

void main()
{
    //local -> world -> camera -> clip-> screen 

    vec3 world_pos = uModel * vec3(aPosition, 1.0); // placing the model in the world
    // here aPosition is the model vertex stuff, fed in by a uniform in quads or other


    vec3 ndc_pos = uViewProjection * world_pos; // translating the model by the view projection aka camera. range is -1 to 1.

    gl_Position = vec4(ndc_pos.xy, 0.0, 1.0); // When a vertex shader outputs gl_Position, it’s in clip space.
    // In our program, our uViewProjection, from camera is just an identity matrix. 

    vec3 tex = uTextureTransform * vec3(aTexCoord, 1.0);
    vTexCoord = tex.xy;

}
