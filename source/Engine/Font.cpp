/**
 * \file
 * \author Rudy Castan
 * \author Jonathan Holmes
 * \author Junseok lee
 * \date 2025 Fall
 * \par CS200 Computer Graphics I
 * \copyright DigiPen Institute of Technology
 */

#include "Font.hpp"

#include "CS200/Image.hpp"
#include "Engine.hpp"
#include "Error.hpp"
#include "Matrix.hpp"
#include "TextureManager.hpp"
#include <algorithm>
#include <sstream>

namespace CS230
{
    Font::Font(const std::filesystem::path& file_name)
    {
        const unsigned int white = 0xFFFFFFFF;
        CS200::Image tempimage(file_name);

        if (*(tempimage.data()) != white)
            throw std::runtime_error("failed to load font : " + file_name.string());

 
        unsigned int check_color = *(tempimage.data());
        unsigned int next_color;

        int height = tempimage.GetSize().y;
        int x = 1; 

        for (int index = 0; index < num_chars; index++) 
        {
            int width = 0;
            do 
            {
                width++;
                
                next_color = *(tempimage.data() + x + width);
            } 
            while (check_color == next_color);

            check_color = next_color;


            char_rects[index].point_1 = { x, 1 };
            char_rects[index].point_2 = { x + width - 1, height };
            x += width;
        }

        font_texture = Engine::GetTextureManager().Load(file_name);
    }

    std::shared_ptr<Texture> Font::PrintToTexture(const std::string& text, CS200::RGBA color)
    {
      
        std::ostringstream oss;
        oss << text << "_0x" << std::hex << color;
        std::string key = oss.str();

 
        auto it = font_cache.find(key);
        if (it != font_cache.end())
        {
            it->second.used_time = Engine::GetWindowEnvironment().FrameCount;
            return it->second.texture;
        }


        int text_width = 0;
        int text_height = 0;

        for (char c : text)
        {
            if (c < ' ' || c > 'z') continue; 

            auto& rect = char_rects[c - ' '];
            text_width += rect.point_2.x - rect.point_1.x + 1;
            text_height = std::max(text_height, rect.point_2.y - rect.point_1.y + 1);
        }

        TextureManager::StartRenderTextureMode(text_width, text_height);

        Math::vec2 pen{ 0.0, 0.0 };

        for (char c : text)
        {
            if (c < ' ' || c > 'z') continue; 

            auto& rect = char_rects[c - ' '];
            int char_width = rect.point_2.x - rect.point_1.x + 1;

            //Math::TransformationMatrix transform = Math::TranslationMatrix(pen);

            Math::vec2 tex_bl{
                rect.point_1.x / double(font_texture->GetSize().x),
                1.0 - rect.point_2.y / double(font_texture->GetSize().y)
            };
            Math::vec2 tex_tr{
                rect.point_2.x / double(font_texture->GetSize().x),
                1.0 - rect.point_1.y / double(font_texture->GetSize().y)
            };

            Math::TransformationMatrix display_matrix = Math::TranslationMatrix{ pen };

            font_texture->Draw(display_matrix, { rect.Left(), rect.Bottom() }, rect.Size(), color);

            pen.x += char_width;
        }

        std::shared_ptr<Texture> rendered_texture = TextureManager::EndRenderTextureMode();

  
        font_cache[key] = { rendered_texture, Engine::GetWindowEnvironment().FrameCount };


        auto current_frame = Engine::GetWindowEnvironment().FrameCount;

        for (auto iter = font_cache.begin(); iter != font_cache.end(); )
        {
            if (current_frame - iter->second.used_time > 60 && iter->second.texture.use_count() == 1)
                iter = font_cache.erase(iter);
            else
                ++iter;
        }

        return rendered_texture;
    }
}
