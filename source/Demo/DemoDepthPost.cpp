/**
 * \file
 * \author Junseok Lee
 * \date 2025 Fall
 * \par CS200 Computer Graphics I
 * \copyright DigiPen Institute of Technology
 */

#include "DemoDepthPost.hpp"

#include "CS200/NDC.hpp"
#include "CS200/RenderingAPI.hpp"
#include "Engine/Engine.hpp"
#include "Engine/Logger.hpp"
#include "Engine/Matrix.hpp"
#include "Engine/Window.hpp"
#include "OpenGL/GL.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <imgui.h>
#include <numeric>
#include <sstream>
#include <stdexcept>

namespace
{
    CS200::RGBA make_color(float r, float g, float b, float a)
    {
        return CS200::pack_color({ r, g, b, a });
    }

    void fill_mat3(const Math::TransformationMatrix& matrix, float* out)
    {
        out[0] = static_cast<float>(matrix[0][0]);
        out[1] = static_cast<float>(matrix[1][0]);
        out[2] = static_cast<float>(matrix[2][0]);
        out[3] = static_cast<float>(matrix[0][1]);
        out[4] = static_cast<float>(matrix[1][1]);
        out[5] = static_cast<float>(matrix[2][1]);
        out[6] = static_cast<float>(matrix[0][2]);
        out[7] = static_cast<float>(matrix[1][2]);
        out[8] = static_cast<float>(matrix[2][2]);
    }

    Math::TransformationMatrix build_transform(const Math::vec2& position, const Math::vec2& size, double rotation)
    {
        return Math::TranslationMatrix(position) * Math::RotationMatrix(rotation) * Math::ScaleMatrix(size);
    }
}

void DemoDepthPost::Load()
{
    CS200::RenderingAPI::SetClearColor(make_color(0.05f, 0.07f, 0.10f, 1.0f));

    createQuad();
    createFullscreenTriangle();
    createWhiteTexture();

    spriteShader = OpenGL::CreateShader(std::filesystem::path{ "Assets/shaders/HW8/sprite.vert" },
                                        std::filesystem::path{ "Assets/shaders/HW8/sprite.frag" });
    chromaticShader = OpenGL::CreateShader(std::filesystem::path{ "Assets/shaders/HW8/fullscreen.vert" },
                                           std::filesystem::path{ "Assets/shaders/HW8/chromatic.frag" });
    vignetteShader = OpenGL::CreateShader(std::filesystem::path{ "Assets/shaders/HW8/fullscreen.vert" },
                                          std::filesystem::path{ "Assets/shaders/HW8/vignette.frag" });
    grainShader = OpenGL::CreateShader(std::filesystem::path{ "Assets/shaders/HW8/fullscreen.vert" },
                                       std::filesystem::path{ "Assets/shaders/HW8/grain.frag" });
    gammaShader = OpenGL::CreateShader(std::filesystem::path{ "Assets/shaders/HW8/fullscreen.vert" },
                                       std::filesystem::path{ "Assets/shaders/HW8/gamma.frag" });

    viewportSize = Engine::GetWindow().GetSize();
    rebuildRenderTargets(viewportSize);
    buildScene(viewportSize);
    updateDrawOrder();
}

void DemoDepthPost::Update()
{
    const auto& environment = Engine::GetWindowEnvironment();
    timeSeconds = static_cast<float>(environment.ElapsedTime);

    const auto current_size = Engine::GetWindow().GetSize();
    if (current_size != viewportSize)
    {
        viewportSize = current_size;
        rebuildRenderTargets(viewportSize);
        buildScene(viewportSize);
    }

    updateAnimatedLayers();
    updateDrawOrder();
}

void DemoDepthPost::Unload()
{
    destroyRenderTargets();
    destroyGeometry();
    destroyWhiteTexture();

    OpenGL::DestroyShader(spriteShader);
    OpenGL::DestroyShader(chromaticShader);
    OpenGL::DestroyShader(vignetteShader);
    OpenGL::DestroyShader(grainShader);
    OpenGL::DestroyShader(gammaShader);
}

void DemoDepthPost::Draw() const
{
    if (viewportSize.x <= 0 || viewportSize.y <= 0)
    {
        return;
    }

    renderSceneToMsaa();
    resolveMsaaToTexture();
    runPostProcessing();
}

void DemoDepthPost::DrawImGui()
{
    if (ImGui::Begin("HW8 - Depth & Post"))
    {
        ImGui::Text("FPS: %d", static_cast<int>(Engine::GetWindowEnvironment().FPS));

        ImGui::SeparatorText("Opaque Draw Order");
        if (ImGui::RadioButton("Front-to-Back", drawOrder == DrawOrder::FrontToBack))
        {
            drawOrder = DrawOrder::FrontToBack;
        }
        if (ImGui::RadioButton("Back-to-Front", drawOrder == DrawOrder::BackToFront))
        {
            drawOrder = DrawOrder::BackToFront;
        }
        if (ImGui::RadioButton("Random", drawOrder == DrawOrder::Random))
        {
            drawOrder = DrawOrder::Random;
        }

        ImGui::SeparatorText("Post Processing");
        ImGui::Checkbox("Chromatic Aberration", &enableChromatic);
        if (enableChromatic)
        {
            ImGui::SliderFloat("Chromatic Strength", &chromaticStrength, 0.0f, 8.0f, "%.2f px");
        }

        ImGui::Checkbox("Vignette", &enableVignette);
        if (enableVignette)
        {
            ImGui::SliderFloat("Vignette Intensity", &vignetteIntensity, 0.0f, 1.0f, "%.2f");
            ImGui::SliderFloat("Vignette Radius", &vignetteRadius, 0.2f, 1.0f, "%.2f");
            ImGui::SliderFloat("Vignette Softness", &vignetteSoftness, 0.02f, 0.6f, "%.2f");
            if (vignetteSoftness > vignetteRadius)
            {
                vignetteSoftness = vignetteRadius;
            }
        }

        ImGui::Checkbox("Film Grain + Scanlines", &enableGrain);
        if (enableGrain)
        {
            ImGui::SliderFloat("Grain Intensity", &grainIntensity, 0.0f, 0.25f, "%.3f");
            ImGui::SliderFloat("Scanline Intensity", &scanlineIntensity, 0.0f, 0.5f, "%.3f");
        }

        ImGui::Checkbox("Gamma Correction", &enableGamma);
        ImGui::SliderFloat("Gamma", &gammaValue, 1.0f, 3.0f, "%.2f");
    }
    ImGui::End();
}

gsl::czstring DemoDepthPost::GetName() const
{
    return "HW8 Depth + Post";
}

void DemoDepthPost::buildScene(Math::ivec2 size)
{
    opaqueItems.clear();
    transparentItems.clear();
    animatedTransparent.clear();

    const double w = static_cast<double>(size.x);
    const double h = static_cast<double>(size.y);

    auto add_opaque = [&](Math::vec2 position, Math::vec2 scale, float depth, CS200::RGBA color, double rotation = 0.0)
    {
        opaqueItems.push_back({ position, scale, rotation, std::clamp(depth, 0.0f, 1.0f), whiteTexture, color });
    };

    auto add_transparent = [&](Math::vec2 position, Math::vec2 scale, float depth, CS200::RGBA color, double rotation = 0.0)
    {
        transparentItems.push_back({ position, scale, rotation, std::clamp(depth, 0.0f, 1.0f), whiteTexture, color });
    };

    const auto sky_top = make_color(0.07f, 0.10f, 0.18f, 1.0f);
    const auto sky_bottom = make_color(0.10f, 0.12f, 0.20f, 1.0f);
    const auto horizon_glow = make_color(0.18f, 0.22f, 0.30f, 1.0f);
    const auto far_buildings = make_color(0.10f, 0.12f, 0.18f, 1.0f);
    const auto mid_buildings = make_color(0.14f, 0.16f, 0.24f, 1.0f);
    const auto ground_color = make_color(0.05f, 0.06f, 0.09f, 1.0f);
    const auto crate_color = make_color(0.26f, 0.22f, 0.18f, 1.0f);
    const auto neon_color = make_color(0.12f, 0.70f, 0.90f, 1.0f);

    add_opaque({ w * 0.5, h * 0.75 }, { w, h * 0.5 }, 0.95f, sky_top);
    add_opaque({ w * 0.5, h * 0.25 }, { w, h * 0.5 }, 0.90f, sky_bottom);
    add_opaque({ w * 0.5, h * 0.55 }, { w, h * 0.10 }, 0.88f, horizon_glow);

    const int far_count = 9;
    for (int i = 0; i < far_count; ++i)
    {
        const double bw = w * 0.08 + (i % 3) * w * 0.015;
        const double bh = h * (0.20 + 0.18 * (i % 4) / 3.0);
        const double x = (i + 0.5) * (w / far_count);
        add_opaque({ x, bh * 0.5 }, { bw, bh }, 0.82f, far_buildings);
    }

    const int mid_count = 6;
    for (int i = 0; i < mid_count; ++i)
    {
        const double bw = w * 0.10 + (i % 2) * w * 0.02;
        const double bh = h * (0.28 + 0.20 * (i % 3) / 2.0);
        const double x = (i + 0.8) * (w / mid_count);
        add_opaque({ x, bh * 0.5 }, { bw, bh }, 0.58f, mid_buildings);
    }

    add_opaque({ w * 0.5, h * 0.10 }, { w, h * 0.20 }, 0.22f, ground_color);

    add_opaque({ w * 0.72, h * 0.20 }, { w * 0.10, h * 0.10 }, 0.10f, crate_color, 0.08);
    add_opaque({ w * 0.82, h * 0.18 }, { w * 0.08, h * 0.08 }, 0.12f, crate_color, -0.05);
    add_opaque({ w * 0.67, h * 0.26 }, { w * 0.04, h * 0.14 }, 0.18f, neon_color);

    const int panel_count = 140;
    const double panel_width = w * 0.42;
    const double panel_height = h * 0.78;
    const double panel_x = w * 0.22;
    const double panel_y = h * 0.48;
    for (int i = 0; i < panel_count; ++i)
    {
        const float t = static_cast<float>(i) / static_cast<float>(panel_count - 1);
        const float depth = 0.14f + 0.68f * t;
        const float shade = 0.09f + 0.07f * std::sin(t * 3.14159f);
        const auto panel_color = make_color(shade, shade * 1.1f, shade * 1.3f, 1.0f);
        const double offset = static_cast<double>((i % 5) - 2);
        add_opaque({ panel_x + offset * 3.0, panel_y + offset * 2.0 }, { panel_width, panel_height }, depth, panel_color);
    }

    const auto fog_color = make_color(0.50f, 0.62f, 0.72f, 0.28f);
    const auto fog_color_close = make_color(0.45f, 0.52f, 0.65f, 0.20f);
    const auto glass_color = make_color(0.30f, 0.85f, 0.90f, 0.18f);

    const std::size_t fog_layer = transparentItems.size();
    add_transparent({ w * 0.5, h * 0.38 }, { w, h * 0.22 }, 0.62f, fog_color);
    animatedTransparent.push_back({ fog_layer, { w * 0.5, h * 0.38 }, 0.4f, static_cast<float>(w * 0.02) });

    const std::size_t fog_layer2 = transparentItems.size();
    add_transparent({ w * 0.5, h * 0.22 }, { w, h * 0.18 }, 0.42f, fog_color_close);
    animatedTransparent.push_back({ fog_layer2, { w * 0.5, h * 0.22 }, 0.6f, static_cast<float>(w * 0.03) });

    add_transparent({ w * 0.78, h * 0.48 }, { w * 0.28, h * 0.70 }, 0.04f, glass_color);
    add_transparent({ w * 0.60, h * 0.52 }, { w * 0.06, h * 0.70 }, 0.55f, make_color(0.20f, 0.80f, 0.90f, 0.24f), 0.05);
    add_transparent({ w * 0.90, h * 0.50 }, { w * 0.05, h * 0.65 }, 0.65f, make_color(0.15f, 0.75f, 0.85f, 0.22f), -0.04);
}

void DemoDepthPost::rebuildRenderTargets(Math::ivec2 size)
{
    destroyRenderTargets();

    GLint max_samples = 0;
    GL::GetIntegerv(GL_MAX_SAMPLES, &max_samples);
    max_samples = std::max(1, max_samples);
    msaaTarget.Samples = std::clamp(msaaTarget.Samples, 1, max_samples);
    msaaTarget.Size = size;

    GL::GenFramebuffers(1, &msaaTarget.Framebuffer);
    GL::BindFramebuffer(GL_FRAMEBUFFER, msaaTarget.Framebuffer);

    GL::GenRenderbuffers(1, &msaaTarget.ColorBuffer);
    GL::BindRenderbuffer(GL_RENDERBUFFER, msaaTarget.ColorBuffer);
    GL::RenderbufferStorageMultisample(GL_RENDERBUFFER, msaaTarget.Samples, GL_RGBA8, size.x, size.y);
    GL::FramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, msaaTarget.ColorBuffer);

    GL::GenRenderbuffers(1, &msaaTarget.DepthBuffer);
    GL::BindRenderbuffer(GL_RENDERBUFFER, msaaTarget.DepthBuffer);
    GL::RenderbufferStorageMultisample(GL_RENDERBUFFER, msaaTarget.Samples, GL_DEPTH24_STENCIL8, size.x, size.y);
    GL::FramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, msaaTarget.DepthBuffer);

    const GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0 };
    GL::DrawBuffers(1, draw_buffers);
    verifyFramebufferComplete("MSAA framebuffer");

    GL::BindFramebuffer(GL_FRAMEBUFFER, 0);
    GL::BindRenderbuffer(GL_RENDERBUFFER, 0);

    resolveTarget = createColorTarget(size);
    postTargets[0] = createColorTarget(size);
    postTargets[1] = createColorTarget(size);
}

void DemoDepthPost::destroyRenderTargets() noexcept
{
    destroyColorTarget(resolveTarget);
    destroyColorTarget(postTargets[0]);
    destroyColorTarget(postTargets[1]);

    if (msaaTarget.ColorBuffer != 0)
    {
        GL::DeleteRenderbuffers(1, &msaaTarget.ColorBuffer);
        msaaTarget.ColorBuffer = 0;
    }
    if (msaaTarget.DepthBuffer != 0)
    {
        GL::DeleteRenderbuffers(1, &msaaTarget.DepthBuffer);
        msaaTarget.DepthBuffer = 0;
    }
    if (msaaTarget.Framebuffer != 0)
    {
        GL::DeleteFramebuffers(1, &msaaTarget.Framebuffer);
        msaaTarget.Framebuffer = 0;
    }
}

void DemoDepthPost::updateDrawOrder()
{
    opaqueOrder.resize(opaqueItems.size());
    std::iota(opaqueOrder.begin(), opaqueOrder.end(), 0);

    switch (drawOrder)
    {
    case DrawOrder::FrontToBack:
        std::sort(opaqueOrder.begin(), opaqueOrder.end(), [&](std::size_t a, std::size_t b)
        {
            return opaqueItems[a].Depth < opaqueItems[b].Depth;
        });
        break;
    case DrawOrder::BackToFront:
        std::sort(opaqueOrder.begin(), opaqueOrder.end(), [&](std::size_t a, std::size_t b)
        {
            return opaqueItems[a].Depth > opaqueItems[b].Depth;
        });
        break;
    case DrawOrder::Random:
        std::shuffle(opaqueOrder.begin(), opaqueOrder.end(), rng);
        break;
    }

    transparentOrder.resize(transparentItems.size());
    std::iota(transparentOrder.begin(), transparentOrder.end(), 0);
    std::sort(transparentOrder.begin(), transparentOrder.end(), [&](std::size_t a, std::size_t b)
    {
        return transparentItems[a].Depth > transparentItems[b].Depth;
    });
}

void DemoDepthPost::updateAnimatedLayers()
{
    for (const auto& layer : animatedTransparent)
    {
        if (layer.Index >= transparentItems.size())
        {
            continue;
        }
        auto& item = transparentItems[layer.Index];
        const double phase = static_cast<double>(timeSeconds) * static_cast<double>(layer.Speed);
        const double drift = std::sin(phase) * static_cast<double>(layer.Amplitude);
        item.Position.x = layer.BasePosition.x + drift;
    }
}

void DemoDepthPost::renderSceneToMsaa() const
{
    GL::BindFramebuffer(GL_FRAMEBUFFER, msaaTarget.Framebuffer);
    GL::Viewport(0, 0, viewportSize.x, viewportSize.y);

    GL::Enable(GL_DEPTH_TEST);
    GL::DepthMask(GL_TRUE);
    GL::Disable(GL_BLEND);

    GL::ClearColor(0.05f, 0.07f, 0.10f, 1.0f);
    GL::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const auto view_projection_matrix = CS200::build_ndc_matrix(viewportSize);
    float view_projection[9] = {};
    fill_mat3(view_projection_matrix, view_projection);

    GL::UseProgram(spriteShader.Shader);
    if (spriteShader.UniformLocations.contains("uViewProjection"))
    {
        GL::UniformMatrix3fv(spriteShader.UniformLocations.at("uViewProjection"), 1, GL_FALSE, view_projection);
    }
    if (spriteShader.UniformLocations.contains("uTexture"))
    {
        GL::Uniform1i(spriteShader.UniformLocations.at("uTexture"), 0);
    }

    GL::BindVertexArray(quadVao);
    for (const auto index : opaqueOrder)
    {
        drawSprite(opaqueItems[index]);
    }

    GL::Enable(GL_BLEND);
    GL::DepthMask(GL_FALSE);
    for (const auto index : transparentOrder)
    {
        drawSprite(transparentItems[index]);
    }
    GL::DepthMask(GL_TRUE);

    GL::BindVertexArray(0);
    GL::BindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DemoDepthPost::resolveMsaaToTexture() const
{
    GL::BindFramebuffer(GL_READ_FRAMEBUFFER, msaaTarget.Framebuffer);
    GL::BindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveTarget.Framebuffer);
    GL::BlitFramebuffer(0, 0, viewportSize.x, viewportSize.y, 0, 0, viewportSize.x, viewportSize.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    GL::BindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    GL::BindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void DemoDepthPost::runPostProcessing() const
{
    OpenGL::TextureHandle current = resolveTarget.Texture;
    int                   ping = 0;

    GL::Disable(GL_DEPTH_TEST);
    GL::DepthMask(GL_FALSE);
    GL::Disable(GL_BLEND);
    GL::BindVertexArray(fullscreenVao);

    if (enableChromatic)
    {
        GL::BindFramebuffer(GL_FRAMEBUFFER, postTargets[ping].Framebuffer);
        GL::Viewport(0, 0, viewportSize.x, viewportSize.y);
        GL::Clear(GL_COLOR_BUFFER_BIT);

        GL::UseProgram(chromaticShader.Shader);
        if (chromaticShader.UniformLocations.contains("uStrength"))
        {
            GL::Uniform1f(chromaticShader.UniformLocations.at("uStrength"), chromaticStrength);
        }
        drawFullscreenPass(chromaticShader, current);
        current = postTargets[ping].Texture;
        ping = 1 - ping;
    }

    if (enableVignette)
    {
        GL::BindFramebuffer(GL_FRAMEBUFFER, postTargets[ping].Framebuffer);
        GL::Viewport(0, 0, viewportSize.x, viewportSize.y);
        GL::Clear(GL_COLOR_BUFFER_BIT);

        GL::UseProgram(vignetteShader.Shader);
        if (vignetteShader.UniformLocations.contains("uIntensity"))
        {
            GL::Uniform1f(vignetteShader.UniformLocations.at("uIntensity"), vignetteIntensity);
        }
        if (vignetteShader.UniformLocations.contains("uRadius"))
        {
            GL::Uniform1f(vignetteShader.UniformLocations.at("uRadius"), vignetteRadius);
        }
        if (vignetteShader.UniformLocations.contains("uSoftness"))
        {
            GL::Uniform1f(vignetteShader.UniformLocations.at("uSoftness"), vignetteSoftness);
        }
        drawFullscreenPass(vignetteShader, current);
        current = postTargets[ping].Texture;
        ping = 1 - ping;
    }

    if (enableGrain)
    {
        GL::BindFramebuffer(GL_FRAMEBUFFER, postTargets[ping].Framebuffer);
        GL::Viewport(0, 0, viewportSize.x, viewportSize.y);
        GL::Clear(GL_COLOR_BUFFER_BIT);

        GL::UseProgram(grainShader.Shader);
        if (grainShader.UniformLocations.contains("uGrainIntensity"))
        {
            GL::Uniform1f(grainShader.UniformLocations.at("uGrainIntensity"), grainIntensity);
        }
        if (grainShader.UniformLocations.contains("uScanlineIntensity"))
        {
            GL::Uniform1f(grainShader.UniformLocations.at("uScanlineIntensity"), scanlineIntensity);
        }
        drawFullscreenPass(grainShader, current);
        current = postTargets[ping].Texture;
    }

    GL::BindFramebuffer(GL_FRAMEBUFFER, 0);
    GL::Viewport(0, 0, viewportSize.x, viewportSize.y);
    GL::Clear(GL_COLOR_BUFFER_BIT);

    GL::UseProgram(gammaShader.Shader);
    if (gammaShader.UniformLocations.contains("uGamma"))
    {
        const float gamma = enableGamma ? gammaValue : 1.0f;
        GL::Uniform1f(gammaShader.UniformLocations.at("uGamma"), gamma);
    }
    drawFullscreenPass(gammaShader, current);

    GL::BindVertexArray(0);
    GL::DepthMask(GL_TRUE);
}

void DemoDepthPost::createQuad()
{
    const float vertices[] = {
        -0.5f, -0.5f, 0.0f, 0.0f,
         0.5f, -0.5f, 1.0f, 0.0f,
         0.5f,  0.5f, 1.0f, 1.0f,
        -0.5f,  0.5f, 0.0f, 1.0f
    };

    const GLuint indices[] = { 0, 1, 2, 2, 3, 0 };

    GL::GenVertexArrays(1, &quadVao);
    GL::BindVertexArray(quadVao);

    GL::GenBuffers(1, &quadVbo);
    GL::BindBuffer(GL_ARRAY_BUFFER, quadVbo);
    GL::BufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    GL::GenBuffers(1, &quadEbo);
    GL::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEbo);
    GL::BufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    GL::EnableVertexAttribArray(0);
    GL::VertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(0));
    GL::EnableVertexAttribArray(1);
    GL::VertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(2 * sizeof(float)));

    GL::BindVertexArray(0);
}

void DemoDepthPost::createFullscreenTriangle()
{
    GL::GenVertexArrays(1, &fullscreenVao);
    GL::BindVertexArray(fullscreenVao);
    GL::BindVertexArray(0);
}

void DemoDepthPost::destroyGeometry() noexcept
{
    if (quadEbo != 0)
    {
        GL::DeleteBuffers(1, &quadEbo);
        quadEbo = 0;
    }
    if (quadVbo != 0)
    {
        GL::DeleteBuffers(1, &quadVbo);
        quadVbo = 0;
    }
    if (quadVao != 0)
    {
        GL::DeleteVertexArrays(1, &quadVao);
        quadVao = 0;
    }
    if (fullscreenVao != 0)
    {
        GL::DeleteVertexArrays(1, &fullscreenVao);
        fullscreenVao = 0;
    }
}

void DemoDepthPost::createWhiteTexture()
{
    std::array<CS200::RGBA, 1> pixel = { CS200::WHITE };
    whiteTexture = OpenGL::CreateTextureFromMemory({ 1, 1 }, pixel, OpenGL::Filtering::NearestPixel, OpenGL::Wrapping::ClampToEdge);
}

void DemoDepthPost::destroyWhiteTexture() noexcept
{
    if (whiteTexture != 0)
    {
        GL::DeleteTextures(1, &whiteTexture);
        whiteTexture = 0;
    }
}

DemoDepthPost::ColorTarget DemoDepthPost::createColorTarget(Math::ivec2 size) const
{
    ColorTarget target{};
    target.Size = size;

    GL::GenFramebuffers(1, &target.Framebuffer);
    GL::BindFramebuffer(GL_FRAMEBUFFER, target.Framebuffer);

    target.Texture = OpenGL::CreateRGBATexture(size, OpenGL::Filtering::Linear, OpenGL::Wrapping::ClampToEdge);
    GL::FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target.Texture, 0);

    const GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0 };
    GL::DrawBuffers(1, draw_buffers);
    verifyFramebufferComplete("color target");

    GL::BindFramebuffer(GL_FRAMEBUFFER, 0);

    return target;
}

void DemoDepthPost::destroyColorTarget(ColorTarget& target) noexcept
{
    if (target.Texture != 0)
    {
        GL::DeleteTextures(1, &target.Texture);
        target.Texture = 0;
    }
    if (target.Framebuffer != 0)
    {
        GL::DeleteFramebuffers(1, &target.Framebuffer);
        target.Framebuffer = 0;
    }
    target.Size = { 0, 0 };
}

void DemoDepthPost::verifyFramebufferComplete(const char* label) const
{
    const GLenum status = GL::CheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status == GL_FRAMEBUFFER_COMPLETE)
    {
        return;
    }

    std::ostringstream message;
    message << label << " incomplete: 0x" << std::hex << status << std::dec;
    Engine::GetLogger().LogError(message.str());
    throw std::runtime_error(message.str());
}

void DemoDepthPost::drawSprite(const RenderItem& item) const
{
    const auto transform = build_transform(item.Position, item.Size, item.Rotation);
    float      model[9] = {};
    fill_mat3(transform, model);

    if (spriteShader.UniformLocations.contains("uModel"))
    {
        GL::UniformMatrix3fv(spriteShader.UniformLocations.at("uModel"), 1, GL_FALSE, model);
    }
    if (spriteShader.UniformLocations.contains("uDepth"))
    {
        GL::Uniform1f(spriteShader.UniformLocations.at("uDepth"), item.Depth);
    }
    if (spriteShader.UniformLocations.contains("uTintColor"))
    {
        const auto color = CS200::unpack_color(item.Tint);
        GL::Uniform4f(spriteShader.UniformLocations.at("uTintColor"), color[0], color[1], color[2], color[3]);
    }

    GL::ActiveTexture(GL_TEXTURE0);
    GL::BindTexture(GL_TEXTURE_2D, item.Texture == 0 ? whiteTexture : item.Texture);
    GL::DrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}

void DemoDepthPost::setPostCommonUniforms(const OpenGL::CompiledShader& shader) const
{
    if (shader.UniformLocations.contains("uInput"))
    {
        GL::Uniform1i(shader.UniformLocations.at("uInput"), 0);
    }
    if (shader.UniformLocations.contains("uTexelSize"))
    {
        GL::Uniform2f(shader.UniformLocations.at("uTexelSize"), 1.0f / static_cast<float>(viewportSize.x), 1.0f / static_cast<float>(viewportSize.y));
    }
    if (shader.UniformLocations.contains("uResolution"))
    {
        GL::Uniform2f(shader.UniformLocations.at("uResolution"), static_cast<float>(viewportSize.x), static_cast<float>(viewportSize.y));
    }
    if (shader.UniformLocations.contains("uTime"))
    {
        GL::Uniform1f(shader.UniformLocations.at("uTime"), timeSeconds);
    }
}

void DemoDepthPost::drawFullscreenPass(const OpenGL::CompiledShader& shader, OpenGL::TextureHandle input) const
{
    setPostCommonUniforms(shader);
    GL::ActiveTexture(GL_TEXTURE0);
    GL::BindTexture(GL_TEXTURE_2D, input);
    GL::DrawArrays(GL_TRIANGLES, 0, 3);
}
