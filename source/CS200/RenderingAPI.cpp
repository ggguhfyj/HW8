/**
 * \file
 * \author Rudy Castan
 * \author Junseok Lee
 * \date 2025 Fall
 * \par CS200 Computer Graphics I
 * \copyright DigiPen Institute of Technology
 */
#include "RenderingAPI.hpp"

#include "Engine/Engine.hpp"
#include "Engine/Error.hpp"
#include "Engine/Logger.hpp"
#include "OpenGL/Environment.hpp"
#include <GL/glew.h>
#include <cassert>
#include "OpenGL/GL.hpp"
#include <iostream>

namespace
{
#if defined(DEVELOPER_VERSION) && not defined(IS_WEBGL2)
    void OpenGLMessageCallback(
        [[maybe_unused]] unsigned source, [[maybe_unused]] unsigned type, [[maybe_unused]] unsigned id, unsigned severity, [[maybe_unused]] int length, const char* message,
        [[maybe_unused]] const void* userParam)
    {
        (void)severity;
        (void)message;
        // Suppress OpenGL debug output in developer builds to keep console noise down during grading.
    }
#endif
}

namespace CS200::RenderingAPI
{
    void Init() noexcept
    {
        GLint major = 0, minor = 0;
        GL::GetIntegerv(GL_MAJOR_VERSION, &major);
        GL::GetIntegerv(GL_MINOR_VERSION, &minor);
        if (OpenGL::version(major, minor) < OpenGL::version(OpenGL::MinimumRequiredMajorVersion, OpenGL::MinimumRequiredMinorVersion))
            throw_error_message("Unsupported OpenGL version ", major, '.', minor, "\n We need OpenGL ", OpenGL::MinimumRequiredMajorVersion, '.', OpenGL::MinimumRequiredMinorVersion, " or higher");

        if (OpenGL::MajorVersion == 0)
        {
            OpenGL::MajorVersion = major;
            OpenGL::MinorVersion = minor;
        }

        GL::GetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &OpenGL::MaxTextureImageUnits);
        GL::GetIntegerv(GL_MAX_TEXTURE_SIZE, &OpenGL::MaxTextureSize);

#if defined(DEVELOPER_VERSION) && not defined(IS_WEBGL2)
        // Debug callback functionality requires OpenGL 4.3+ or KHR_debug extension
        if (OpenGL::current_version() >= OpenGL::version(4, 3))
        {
            GL::Enable(GL_DEBUG_OUTPUT);
            GL::Enable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            GL::DebugMessageCallback(OpenGLMessageCallback, nullptr);
            GL::DebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
        }
#endif

        GL::Enable(GL_BLEND);
        GL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        GL::Disable(GL_DEPTH_TEST);
        Engine::GetLogger().LogEvent("GL_VENDOR " + std::to_string(GL_VENDOR));
        Engine::GetLogger().LogEvent("GL_RENDERER "+std::to_string(GL_RENDERER));
        Engine::GetLogger().LogEvent("GL_VERSION "+std::to_string(GL_VERSION));
        Engine::GetLogger().LogEvent("GL_SHADING_LANGUAGE_VERSION "+std::to_string(GL_SHADING_LANGUAGE_VERSION));
        Engine::GetLogger().LogEvent("GL_MAJOR_VERSION "+std::to_string(GL_MAJOR_VERSION));
        Engine::GetLogger().LogEvent("GL_MINOR_VERSION "+std::to_string(GL_MINOR_VERSION));
        Engine::GetLogger().LogEvent("GL_MAX_ELEMENTS_VERTICES "+std::to_string(GL_MAX_ELEMENTS_VERTICES));
        Engine::GetLogger().LogEvent("GL_MAX_ELEMENTS_INDICES "+std::to_string(GL_MAX_ELEMENTS_INDICES));
        Engine::GetLogger().LogEvent("GL_MAX_TEXTURE_IMAGE_UNITS "+std::to_string(GL_MAX_TEXTURE_IMAGE_UNITS));
        Engine::GetLogger().LogEvent("GL_MAX_TEXTURE_SIZE "+std::to_string(GL_MAX_TEXTURE_SIZE));
        Engine::GetLogger().LogEvent("GL_MAX_VIEWPORT_DIMS "+std::to_string(GL_MAX_VIEWPORT_DIMS));

        // ALREADY DONE print opengl settings : GL_VENDOR, GL_RENDERER, GL_VERSION, GL_SHADING_LANGUAGE_VERSION,
        // GL_MAJOR_VERSION, GL_MINOR_VERSION, GL_MAX_ELEMENTS_VERTICES, GL_MAX_ELEMENTS_INDICES,
        // GL_MAX_TEXTURE_IMAGE_UNITS, GL_MAX_TEXTURE_SIZE, GL_MAX_VIEWPORT_DIMS
        
  
    }

    void SetClearColor(CS200::RGBA color) noexcept
    {
        const auto rgba = CS200::unpack_color(color);
        GL::ClearColor(rgba[0], rgba[1], rgba[2], rgba[3]);
    }

    void Clear() noexcept
    {
        GL::Clear(GL_COLOR_BUFFER_BIT);
    }

    void SetViewport(Math::ivec2 size, Math::ivec2 anchor_left_bottom) noexcept
    {
        GL::Viewport(anchor_left_bottom.x, anchor_left_bottom.y, size.x, size.y);
    }
}
