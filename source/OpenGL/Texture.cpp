/**
 * \file
 * \author Rudy Castan
 * \author JUNSEOK LEE
 * \date 2025 Fall
 * \par CS200 Computer Graphics I
 * \copyright DigiPen Institute of Technology
 */
#include "Texture.hpp"
#include "CS200/Image.hpp"
#include "Environment.hpp"
#include "GL.hpp"

namespace OpenGL
{
   TextureHandle CreateTextureFromImage(const CS200::Image& image, Filtering filtering, Wrapping wrapping) noexcept
    {
        auto size = image.GetSize();
    
       auto handle =  CreateTextureFromMemory(size,  std::span<const CS200::RGBA>(image.data(),static_cast<size_t>(size.x * size.y))  ,filtering,   wrapping);
       return handle;
    
    
    }

    TextureHandle CreateTextureFromMemory(Math::ivec2 size, std::span<const CS200::RGBA> colors,Filtering filtering, Wrapping wrapping) noexcept
    {
        TextureHandle handle{};
        GL::GenTextures(1, &handle);
        GL::BindTexture(GL_TEXTURE_2D, handle);

        GL::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(filtering));
        GL::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(filtering));


        GL::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<GLint>(wrapping));
        GL::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<GLint>(wrapping));
        //https://learn.microsoft.com/en-us/windows/win32/opengl/gltexparameteri


        // Upload texture data to GPU
        GL::TexImage2D(GL_TEXTURE_2D,
                       0,
                       GL_RGBA8,             // internal format
                       size.x,
                       size.y,
                       0,
                       GL_RGBA,              // data format
                       GL_UNSIGNED_BYTE,     // data type (check your RGBA definition)
                       colors.data());       // pointer to pixel data

        // Unbind
        GL::BindTexture(GL_TEXTURE_2D, 0);

        return handle;
    }




    TextureHandle CreateRGBATexture(Math::ivec2 size, Filtering filtering, Wrapping wrapping) noexcept // make empty
    {
        TextureHandle texture_handle = 0;
        GL::GenTextures(1, &texture_handle);
        GL::BindTexture(GL_TEXTURE_2D, texture_handle);
        GL::TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        GL::BindTexture(GL_TEXTURE_2D, 0);

        SetFiltering(texture_handle, filtering);
        SetWrapping(texture_handle, wrapping);

        return texture_handle;
    }

    void SetFiltering(TextureHandle texture_handle, Filtering filtering) noexcept
    {
        GL::BindTexture(GL_TEXTURE_2D, texture_handle);
        GL::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(filtering));
        GL::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(filtering));
        GL::BindTexture(GL_TEXTURE_2D, 0);
    }

    void SetWrapping(TextureHandle texture_handle, Wrapping wrapping, TextureCoordinate coord) noexcept
    {
        
        GL::BindTexture(GL_TEXTURE_2D, texture_handle);
        if(coord == TextureCoordinate::S)
        {
            GL::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<GLint>(wrapping));
        }
        else if(coord == TextureCoordinate::T)
        {
            GL::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<GLint>(wrapping));
        }
        else
        {
            GL::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<GLint>(wrapping));
            GL::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<GLint>(wrapping));
        }

        GL::BindTexture(GL_TEXTURE_2D, 0);
    }
}

