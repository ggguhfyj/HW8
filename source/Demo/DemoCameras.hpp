/**
 * \file
 * \author Jonathan Holmes
 * \date 2025 Fall
 * \par CS200 Computer Graphics I
 * \copyright DigiPen Institute of Technology
 */

#pragma once

#include "CS200/RGBA.hpp"
#include "Engine/GameState.hpp"
#include "Engine/Matrix.hpp"
#include "Engine/Vec2.hpp"
#include "OpenGL/GLTypes.hpp"
#include "OpenGL/Shader.hpp"
#include <array>
#include <gsl/gsl>
#include <string>
#include <vector>

class DemoCameras : public CS230::GameState
{
public:
    void          Load() override;
    void          Update() override;
    void          Unload() override;
    void          Draw() const override;
    void          DrawImGui() override;
    gsl::czstring GetName() const override;

private:
    enum class CameraMode
    {
        Follow,
        FirstPerson
    };

    struct Player
    {
        Math::vec2 position{};
        double     rotation{};
        double     speed{ 240.0 };
        CS200::RGBA color{};
        std::string name{};
    };

    struct Camera
    {
        Math::vec2  position{};
        double      rotation{};
        double      zoom{ 1.0 };
        double      targetZoom{ 1.0 };
        CameraMode  mode{ CameraMode::Follow };
        std::string label{};
    };

    struct Viewport
    {
        Math::ivec2 origin{};
        Math::ivec2 size{};
    };

    struct ColoredVertex
    {
        std::array<float, 2> position{};
        std::array<float, 4> color{};
    };

    class PrimitiveRenderer
    {
    public:
        void Init();
        void Shutdown();
        void Draw(GLenum mode, gsl::span<const ColoredVertex> vertices, const Math::TransformationMatrix& mvp, float point_size = 4.0f) const;

    private:
        std::array<float, 9> toFloatArray(const Math::TransformationMatrix& matrix) const;

    private:
        OpenGL::CompiledShader shader{};
        GLuint                 vao{ 0 };
        GLuint                 vbo{ 0 };
        GLint                  mvpLocation{ -1 };
        GLint                  pointSizeLocation{ -1 };
    };

private:
    void updatePlayers(double delta_time);
    void updateCameraState(std::size_t index, double delta_time);
    void buildViewportLayout(Math::ivec2 window_size);
    Math::TransformationMatrix buildViewProjection(const Camera& camera, Math::ivec2 viewport_size) const;

    void drawWorld(const Math::TransformationMatrix& view_projection) const;
    void drawPlayers(const Math::TransformationMatrix& view_projection) const;
    void drawSeparators() const;
    void setupWorldGeometry();

private:
    Math::vec2               worldSize{ 2400.0, 1800.0 };
    int                      viewportCount{ 2 };
    int                      activeViewport{ 0 };
    bool                     showGrid{ true };
    std::vector<Player>      players{};
    std::vector<Camera>      cameras{};
    std::vector<Viewport>    viewports{};
    std::vector<ColoredVertex> pointField{};
    std::vector<ColoredVertex> boundaryLines{};
    std::vector<ColoredVertex> pathStrip{};
    std::vector<ColoredVertex> lakeLoop{};
    std::vector<ColoredVertex> mountainTriangles{};
    std::vector<ColoredVertex> sunFan{};
    std::vector<ColoredVertex> riverStrip{};

    mutable PrimitiveRenderer primitiveRenderer{};
};
