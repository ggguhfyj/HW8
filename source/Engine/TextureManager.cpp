/**
 * \file
 * \author Rudy Castan
 * \author Jonathan Holmes
 * \author JUNSEOK LEE
 * \date 2025 Fall
 * \par CS200 Computer Graphics I
 * \copyright DigiPen Institute of Technology
 */

#include "TextureManager.hpp"
#include "CS200/IRenderer2D.hpp"
#include "CS200/NDC.hpp"
#include "Engine.hpp"
#include "Logger.hpp"
#include "OpenGL/GL.hpp"
#include "Texture.hpp"

namespace CS230
{
    std::shared_ptr<Texture> TextureManager::Load(const std::filesystem::path& file_name)
    {

        auto it = texture_cache.find(file_name);

        if (it != texture_cache.end())
        {
            return it->second;
        }
        else 
        {

            std::shared_ptr<Texture> newtexture(new Texture(file_name)); // calls the constructor with the arguement

            texture_cache[file_name] = newtexture;

            loaded_textures.push_back(newtexture);

            Engine::GetLogger().LogDebug("Loaded texture for first time : " + file_name.string());
            return newtexture;
        }

    }

    void TextureManager::Unload()
    {
        Engine::GetLogger().LogDebug("unloading textures");

        loaded_textures.clear();

        texture_cache.clear();


    }

    void TextureManager::StartRenderTextureMode(int width, int height)
    {

        GL::GetIntegerv(GL_VIEWPORT, gSavedState.viewport);
        GL::GetFloatv(GL_COLOR_CLEAR_VALUE, gSavedState.clearColor); // save state

        Engine::GetRenderer2D().EndScene();//end the current scene after we save the state into a struct 
        //flush out 

        gSavedState.createdFb = OpenGL::CreateFramebufferWithColor({width,height});// create a new frame buffer with the width and height
        GL::BindFramebuffer(GL_FRAMEBUFFER, gSavedState.createdFb.Framebuffer);
        GL::Viewport(0, 0, width, height); //create a new VIEWPORT

        gSavedState.size_fb = {width,height};

        // Clear to transparent black
        GL::ClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        GL::Clear(GL_COLOR_BUFFER_BIT);//color buffer overrited with clear color. 



        const auto ndc_matrix = Math::ScaleMatrix({ 1.0, 1.0 }) * CS200::build_ndc_matrix({width, height});

        Engine::GetRenderer2D().BeginScene(ndc_matrix); // we use a flip matrix to go from the

        Engine::GetLogger().LogDebug("Entered render-to-texture mode");

    }

    std::shared_ptr<Texture> TextureManager::EndRenderTextureMode()
    {
        Engine::GetRenderer2D().EndScene();// end the texture scene

        GL::BindFramebuffer(GL_FRAMEBUFFER, 0);

        GL::Viewport(gSavedState.viewport[0], gSavedState.viewport[1],gSavedState.viewport[2], gSavedState.viewport[3]);//restore the viewport that we stored

        GL::ClearColor(gSavedState.clearColor[0], gSavedState.clearColor[1],gSavedState.clearColor[2], gSavedState.clearColor[3]);//restore the clear color that we stored
        


        auto ndc_matrix = CS200::build_ndc_matrix({gSavedState.viewport[2], gSavedState.viewport[3]});
        Engine::GetRenderer2D().BeginScene(ndc_matrix);// begin the scene with the default transformation matrix


        GL::DeleteFramebuffers(1, &gSavedState.createdFb.Framebuffer);// we no longer need this as we are not drawing it

        Engine::GetLogger().LogDebug("Exited render-to-texture mode");
        return std::shared_ptr<Texture>(new Texture(gSavedState.createdFb.ColorAttachment,gSavedState.size_fb));// imma be honest I have no idea why we do this
    }
}
