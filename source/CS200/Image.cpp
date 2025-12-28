/**
 * \file
 * \author Rudy Castan
 * \author Junseok Lee
 * \date 2025 Fall
 * \par CS200 Computer Graphics I
 * \copyright DigiPen Institute of Technology
 */
#include "Image.hpp"

#include "Engine/Error.hpp"
#include "Engine/Path.hpp"

#include <stb_image.h>

namespace CS200
{
    Image::Image(const std::filesystem::path& image_path, bool flip_vertical)
    {
        const std::filesystem::path path_image = assets::locate_asset(image_path);

        stbi_set_flip_vertically_on_load(flip_vertical);
        int x = 0, y = 0;
        int files_num_channels = 0;
        constexpr int desired_channels = 4; // RGBA

        pixeldata = stbi_load(path_image.string().c_str(),&x,&y,&files_num_channels,desired_channels);

        if(pixeldata == nullptr)
        {
            throw std::runtime_error("failed to load image : " + path_image.string());
        }
        size = {x,y};
    }

   Image::Image(Image&& temporary) noexcept
    : pixeldata(temporary.pixeldata), size(temporary.size)
{
    // Leave temporary in a safe empty state
    temporary.pixeldata = nullptr;
    temporary.size = {0,0};
}

    Image& Image::operator=(Image&& temporary) noexcept
    {

        pixeldata = std::move(temporary.pixeldata);

        size = std::move(temporary.size);
        return *this;
    }

    Image::~Image()
    {
        if(pixeldata != nullptr)
        {
             stbi_image_free(pixeldata);
        }
    }

    const RGBA* Image::data() const noexcept
    {

        return reinterpret_cast<const RGBA*>(pixeldata);
    }

    RGBA* Image::data() noexcept
    {
        return reinterpret_cast<RGBA*>(pixeldata);
    }

    Math::ivec2 Image::GetSize() const noexcept
    {
        return size;
    }



}
