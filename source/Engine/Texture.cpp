/**
 * \file
 * \author Rudy Castan
 * \author Jonathan Holmes
 * \author Junseok Lee
 * \date 2025 Fall
 * \par CS200 Computer Graphics I
 * \copyright DigiPen Institute of Technology
 */

#include "Texture.hpp"

#include "CS200/IRenderer2D.hpp"
#include "CS200/Image.hpp"
#include "Engine.hpp"
#include "OpenGL/GL.hpp"

namespace CS230
{

    void CS230::Texture::Draw(const Math::TransformationMatrix& display_matrix, unsigned int color)
    {
        // probably using the texture handle

        auto& renderer2D = Engine::GetRenderer2D();
        Math::TranslationMatrix shift(Math::vec2{ 0.5, 0.5 }); // so that the bottom left corner aligns at the origin. (this is an open gl thing)
        Math::ScaleMatrix       scale(Math::vec2{ static_cast<double>(size.x), static_cast<double>(size.y) }); // makes the quad the same size as the texture

        auto transform = display_matrix * scale * shift;
        // transform: out quad vertices have the center at 0 and extents 0.5 to either side. to make this fit with the texture coordinate system,
        // we need to pass a transformation matrix that shifts and scales it so that they perfectly overlap eachother.

        renderer2D.DrawQuad(transform, textureHandle, { 0.0, 0.0 }, { 1.0,  1.0 }, color);

        
    }

    void Texture::Draw(const Math::TransformationMatrix& display_matrix, Math::ivec2 texel_position, Math::ivec2 frame_size, unsigned int color)
    {
        auto& renderer2D = Engine::GetRenderer2D();

        Math::TranslationMatrix shift(Math::vec2{ 0.5, 0.5 }); // same thing, same idea
        Math::ScaleMatrix scale(Math::vec2{ static_cast<double>(frame_size.x), static_cast<double>(frame_size.y) });


        auto transform = display_matrix * scale * shift;

        Math::vec2 Bottom_Left  = { static_cast<double>(texel_position.x) / static_cast<double>(size.x), static_cast<double>(size.y - texel_position.y - frame_size.y) / static_cast<double>(size.y) };// given texel position should 
        Math::vec2 Top_Right    = { static_cast<double>(texel_position.x + frame_size.x) / static_cast<double>(size.x), static_cast<double>(size.y - texel_position.y) / static_cast<double>(size.y) };
        renderer2D.DrawQuad(transform, textureHandle, Bottom_Left, Top_Right , color);
        //

        //range should be [0 1] range and have the origin at the bottom left of the texture
    }

    Math::ivec2 Texture::GetSize() const
    {
        return size;
    }

    Texture::~Texture()
    {
         if (textureHandle != 0)
        {
            GL::DeleteTextures(1, &textureHandle);
        }
    }

    Texture::Texture(const std::filesystem::path& file_name)
    {
        constexpr bool flip_image = true;
        CS200::Image   image(file_name, flip_image);

        size = image.GetSize();

        textureHandle = OpenGL::CreateTextureFromImage(image);
    }


     Texture::Texture(OpenGL::TextureHandle given_texture, Math::ivec2 the_size) : textureHandle(given_texture), size(the_size)
    {
    }

    Texture::Texture(Texture&& temporary) noexcept// move assignment operator
    {
        // return value optimization and shi....
        textureHandle = temporary.textureHandle;
        size = temporary.size;

        temporary.textureHandle = 0;
        temporary.size          = { 0, 0 };
    }

    Texture& Texture::operator=(Texture&& temporary) noexcept
    {
        std::swap(textureHandle, temporary.textureHandle);
        std::swap(size, temporary.size);
        return *this;
    }
}