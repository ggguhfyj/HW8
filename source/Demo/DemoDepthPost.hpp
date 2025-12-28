/**
 * \file
 * \author JUNSEOK LEE
 * \date 2025 December 29
 * \note as the days dwindle down to a precious few
 * \par CS200 Computer Graphics I
 * \copyright DigiPen Institute of Technology
 */
#pragma once

#include "CS200/RGBA.hpp"
#include "Engine/GameState.hpp"
#include "Engine/Vec2.hpp"
#include "OpenGL/Framebuffer.hpp"
#include "OpenGL/Shader.hpp"
#include "OpenGL/Texture.hpp"

#include <gsl/gsl>
#include <random>
#include <vector>

class DemoDepthPost final : public CS230::GameState
{
public:
    void          Load() override;
    void          Update() override;
    void          Unload() override;
    void          Draw() const override;
    void          DrawImGui() override;
    gsl::czstring GetName() const override;

private:
    enum class DrawOrder
    {
        FrontToBack,
        BackToFront,
        Random
    };

    struct RenderItem
    {
        Math::vec2              Position;
        Math::vec2              Size;
        double                  Rotation;
        float                   Depth;
        OpenGL::TextureHandle   Texture;
        CS200::RGBA             Tint;
    };

    struct AnimatedLayer
    {
        std::size_t Index;
        Math::vec2  BasePosition;
        float       Speed;
        float       Amplitude;
    };

    struct MsaaTarget
    {
        OpenGL::FramebufferHandle Framebuffer = 0;
        OpenGL::Handle            ColorBuffer = 0;
        OpenGL::Handle            DepthBuffer = 0;
        Math::ivec2               Size{ 0, 0 };
        int                       Samples = 4;
    };

    struct ColorTarget
    {
        OpenGL::FramebufferHandle Framebuffer = 0;
        OpenGL::TextureHandle     Texture = 0;
        Math::ivec2               Size{ 0, 0 };
    };

private:
    void buildScene(Math::ivec2 size);
    void rebuildRenderTargets(Math::ivec2 size);
    void destroyRenderTargets() noexcept;
    void updateDrawOrder();
    void updateAnimatedLayers();

    void renderSceneToMsaa() const;
    void resolveMsaaToTexture() const;
    void runPostProcessing() const;

    void createQuad();
    void createFullscreenTriangle();
    void destroyGeometry() noexcept;
    void createWhiteTexture();
    void destroyWhiteTexture() noexcept;

    ColorTarget createColorTarget(Math::ivec2 size) const;
    void        destroyColorTarget(ColorTarget& target) noexcept;
    void        verifyFramebufferComplete(const char* label) const;

    void drawSprite(const RenderItem& item) const;
    void setPostCommonUniforms(const OpenGL::CompiledShader& shader) const;
    void drawFullscreenPass(const OpenGL::CompiledShader& shader, OpenGL::TextureHandle input) const;

private:
    std::vector<RenderItem>      opaqueItems;
    std::vector<RenderItem>      transparentItems;
    std::vector<std::size_t>     opaqueOrder;
    std::vector<std::size_t>     transparentOrder;
    std::vector<AnimatedLayer>   animatedTransparent;

    DrawOrder                    drawOrder = DrawOrder::FrontToBack;
    std::mt19937                 rng{ std::random_device{}() };

    OpenGL::TextureHandle        whiteTexture = 0;

    OpenGL::CompiledShader       spriteShader;
    OpenGL::CompiledShader       chromaticShader;
    OpenGL::CompiledShader       vignetteShader;
    OpenGL::CompiledShader       grainShader;
    OpenGL::CompiledShader       gammaShader;

    OpenGL::Handle               quadVao = 0;
    OpenGL::Handle               quadVbo = 0;
    OpenGL::Handle               quadEbo = 0;
    OpenGL::Handle               fullscreenVao = 0;

    MsaaTarget                   msaaTarget;
    ColorTarget                  resolveTarget;
    ColorTarget                  postTargets[2];

    Math::ivec2                  viewportSize{ 0, 0 };

    bool                         enableChromatic = true;
    bool                         enableVignette = true;
    bool                         enableGrain = true;
    bool                         enableGamma = true;

    float                        chromaticStrength = 2.0f;
    float                        vignetteIntensity = 0.35f;
    float                        vignetteRadius = 0.75f;
    float                        vignetteSoftness = 0.35f;
    float                        grainIntensity = 0.08f;
    float                        scanlineIntensity = 0.12f;
    float                        gammaValue = 2.2f;

    float                        timeSeconds = 0.0f;
};
