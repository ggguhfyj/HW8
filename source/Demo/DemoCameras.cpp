/**
 * \file
 * \author Jonathan Holmes
 * \date 2025 Fall
 * \par CS200 Computer Graphics I
 * \copyright DigiPen Institute of Technology
 */

#include "DemoCameras.hpp"

#include "CS200/NDC.hpp"
#include "CS200/Renderer2DUtils.hpp"
#include "CS200/RenderingAPI.hpp"
#include "DemoFramebuffer.hpp"
#include "DemoShapes.hpp"
#include "DemoText.hpp"
#include "Engine/Engine.hpp"
#include "Engine/GameStateManager.hpp"
#include "Engine/Input.hpp"
#include "Engine/Path.hpp"
#include "Engine/Random.hpp"
#include "Engine/Window.hpp"
#include "OpenGL/GL.hpp"
#include <algorithm>
#include <cmath>
#include <imgui.h>
#include <numbers>

namespace
{
    template <typename T>
    void ease_to_target(T& current, const T& target, double delta_time, double weight)
    {
        const auto easing = std::min(delta_time * weight, 1.0);
        current += static_cast<T>(easing * (target - current));
    }

    std::array<float, 4> rgba_to_float4(CS200::RGBA color)
    {
        return CS200::unpack_color(color);
    }
}

// --------------------------- PrimitiveRenderer -----------------------------

void DemoCameras::PrimitiveRenderer::Init()
{
    shader = OpenGL::CreateShader(
        assets::locate_asset("Assets/shaders/CameraDemo/primitive.vert"),
        assets::locate_asset("Assets/shaders/CameraDemo/primitive.frag"));

    mvpLocation       = shader.UniformLocations.contains("uMVP") ? shader.UniformLocations.at("uMVP") : -1;
    pointSizeLocation = shader.UniformLocations.contains("uPointSize") ? shader.UniformLocations.at("uPointSize") : -1;

    GL::GenVertexArrays(1, &vao);
    GL::GenBuffers(1, &vbo);

    GL::BindVertexArray(vao);
    GL::BindBuffer(GL_ARRAY_BUFFER, vbo);
    GL::EnableVertexAttribArray(0);
    GL::VertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex), reinterpret_cast<void*>(0));
    GL::EnableVertexAttribArray(1);
    GL::VertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex), reinterpret_cast<void*>(sizeof(float) * 2));
    GL::BindVertexArray(0);
}

void DemoCameras::PrimitiveRenderer::Shutdown()
{
    if (vao)
    {
        GL::DeleteVertexArrays(1, &vao);
        vao = 0;
    }
    if (vbo)
    {
        GL::DeleteBuffers(1, &vbo);
        vbo = 0;
    }
    OpenGL::DestroyShader(shader);
}

std::array<float, 9> DemoCameras::PrimitiveRenderer::toFloatArray(const Math::TransformationMatrix& matrix) const
{
    return CS200::Renderer2DUtils::to_opengl_mat3(matrix);
}

void DemoCameras::PrimitiveRenderer::Draw(GLenum mode, gsl::span<const ColoredVertex> vertices, const Math::TransformationMatrix& mvp, float point_size) const
{
    if (vertices.empty() || !vao || !shader.Shader)
        return;

    GL::UseProgram(shader.Shader);
    GL::BindVertexArray(vao);
    GL::BindBuffer(GL_ARRAY_BUFFER, vbo);
    GL::BufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(ColoredVertex)), vertices.data(), GL_DYNAMIC_DRAW);

    if (mvpLocation >= 0)
    {
        const auto matrix_data = toFloatArray(mvp);
        GL::UniformMatrix3fv(mvpLocation, 1, GL_FALSE, matrix_data.data());
    }

    if (pointSizeLocation >= 0)
    {
        GL::Uniform1f(pointSizeLocation, point_size);
    }

    GL::DrawArrays(mode, 0, static_cast<GLsizei>(vertices.size()));
    GL::BindVertexArray(0);
    GL::UseProgram(0);
}

// ------------------------------- DemoCameras -------------------------------

void DemoCameras::Load()
{
    CS200::RenderingAPI::SetClearColor(0x0F1118FF);

    players = {
        { Math::vec2{ 400.0, 400.0 }, 0.0, 240.0, 0xFF7F50FF, "Navigator A" },
        { Math::vec2{ 1800.0, 300.0 }, std::numbers::pi * 0.5, 240.0, 0x70C1FFFF, "Navigator B" },
        { Math::vec2{ 600.0, 1400.0 }, 0.0, 240.0, 0xFFCC00FF, "Navigator C" },
        { Math::vec2{ 1800.0, 1300.0 }, std::numbers::pi, 240.0, 0x9F7AFFFF, "Navigator D" }
    };

    cameras.resize(players.size());
    for (std::size_t i = 0; i < players.size(); ++i)
    {
        cameras[i].position   = players[i].position;
        cameras[i].rotation   = players[i].rotation;
        cameras[i].targetZoom = 1.2;
        cameras[i].zoom       = 1.2;
        cameras[i].label      = "Viewport " + std::to_string(i + 1);
    }

    buildViewportLayout(Engine::GetWindow().GetSize());
    setupWorldGeometry();
    primitiveRenderer.Init();
}

void DemoCameras::Update()
{
    const auto& input       = Engine::GetInput();
    const auto& environment = Engine::GetWindowEnvironment();

    // Viewport switching (number keys 1-4)
    if (input.KeyJustPressed(CS230::Input::Keys::One))
        viewportCount = 1;
    if (input.KeyJustPressed(CS230::Input::Keys::Two))
        viewportCount = 2;
    if (input.KeyJustPressed(CS230::Input::Keys::Three))
        viewportCount = 3;
    if (input.KeyJustPressed(CS230::Input::Keys::Four))
        viewportCount = 4;

    viewportCount  = std::clamp(viewportCount, 1, 4);
    activeViewport = activeViewport % viewportCount;

    // Cycle active viewport
    if (input.KeyJustPressed(CS230::Input::Keys::Tab))
    {
        activeViewport = (activeViewport + 1) % viewportCount;
    }

    // Toggle camera mode for the active viewport
    if (input.KeyJustPressed(CS230::Input::Keys::Space))
    {
        auto& camera = cameras[static_cast<std::size_t>(activeViewport)];
        camera.mode  = (camera.mode == CameraMode::Follow) ? CameraMode::FirstPerson : CameraMode::Follow;
    }

    // Adjust zoom for active viewport
    auto& active_camera = cameras[static_cast<std::size_t>(activeViewport)];
    if (input.KeyDown(CS230::Input::Keys::Q))
        active_camera.targetZoom = std::max(0.5, active_camera.targetZoom - environment.DeltaTime * 1.2);
    if (input.KeyDown(CS230::Input::Keys::E))
        active_camera.targetZoom = std::min(3.5, active_camera.targetZoom + environment.DeltaTime * 1.2);

    updatePlayers(environment.DeltaTime);

    buildViewportLayout(Engine::GetWindow().GetSize());
}

void DemoCameras::Unload()
{
    primitiveRenderer.Shutdown();
}

void DemoCameras::Draw() const
{
    CS200::RenderingAPI::Clear();

    auto& renderer = Engine::GetRenderer2D();

    for (std::size_t i = 0; i < static_cast<std::size_t>(viewportCount); ++i)
    {
        const auto& viewport = viewports[i];
        CS200::RenderingAPI::SetViewport(viewport.size, viewport.origin);

        const auto view_projection = buildViewProjection(cameras[i], viewport.size);
        renderer.BeginScene(view_projection);
        drawWorld(view_projection);
        drawPlayers(view_projection);
        renderer.EndScene();
    }

    // Draw separators using full viewport
    const auto window_size = Engine::GetWindow().GetSize();
    CS200::RenderingAPI::SetViewport(window_size);
    renderer.BeginScene(CS200::build_ndc_matrix(window_size));
    drawSeparators();
    renderer.EndScene();
}

void DemoCameras::DrawImGui()
{
    if (ImGui::Begin("Camera Playground"))
    {
        ImGui::Text("Viewport Count");
        if (ImGui::RadioButton("1", viewportCount == 1)) viewportCount = 1;
        ImGui::SameLine();
        if (ImGui::RadioButton("2", viewportCount == 2)) viewportCount = 2;
        ImGui::SameLine();
        if (ImGui::RadioButton("3", viewportCount == 3)) viewportCount = 3;
        ImGui::SameLine();
        if (ImGui::RadioButton("4", viewportCount == 4)) viewportCount = 4;

        ImGui::SeparatorText("Active View");
        ImGui::Text("Active Viewport: %d", activeViewport + 1);
        ImGui::Text("Mode: %s", cameras[static_cast<std::size_t>(activeViewport)].mode == CameraMode::Follow ? "Follow" : "First Person");
        ImGui::SliderFloat("Zoom", reinterpret_cast<float*>(&cameras[static_cast<std::size_t>(activeViewport)].targetZoom), 0.5f, 3.5f);
        ImGui::Checkbox("Show Grid", &showGrid);

        if (ImGui::Button("Toggle Mode (Space)"))
        {
            auto& camera = cameras[static_cast<std::size_t>(activeViewport)];
            camera.mode  = (camera.mode == CameraMode::Follow) ? CameraMode::FirstPerson : CameraMode::Follow;
        }

        ImGui::SeparatorText("Controls");
        ImGui::BulletText("Viewport 1 (WASD)   - forward/back + turn");
        ImGui::BulletText("Viewport 2 (Arrows) - forward/back + turn");
        ImGui::BulletText("Viewport 3 (IJKL)   - forward/back + turn");
        ImGui::BulletText("Viewport 4 (Numpad 8/5/4/6)");
        ImGui::BulletText("Tab cycles active viewport, Space toggles camera mode, Q/E zoom");
        ImGui::BulletText("Number keys 1-4 change viewport layout");

        ImGui::SeparatorText("Switch Demo");
        if (ImGui::Button("Switch to Demo Shapes"))
        {
            Engine::GetGameStateManager().PopState();
            Engine::GetGameStateManager().PushState<DemoShapes>();
        }
        if (ImGui::Button("Switch to Demo Framebuffer"))
        {
            Engine::GetGameStateManager().PopState();
            Engine::GetGameStateManager().PushState<DemoFramebuffer>();
        }
        if (ImGui::Button("Switch to Demo Text"))
        {
            Engine::GetGameStateManager().PopState();
            Engine::GetGameStateManager().PushState<DemoText>();
        }
    }
    ImGui::End();
}

gsl::czstring DemoCameras::GetName() const
{
    return "HW7 - Cameras and Viewports";
}

void DemoCameras::updatePlayers(double delta_time)
{
    struct ControlScheme
    {
        CS230::Input::Keys forward;
        CS230::Input::Keys backward;
        CS230::Input::Keys turnLeft;
        CS230::Input::Keys turnRight;
    };

    static const std::array<ControlScheme, 4> CONTROL_SCHEMES = {
        ControlScheme{ CS230::Input::Keys::W, CS230::Input::Keys::S, CS230::Input::Keys::A, CS230::Input::Keys::D },
        ControlScheme{ CS230::Input::Keys::Up, CS230::Input::Keys::Down, CS230::Input::Keys::Left, CS230::Input::Keys::Right },
        ControlScheme{ CS230::Input::Keys::I, CS230::Input::Keys::K, CS230::Input::Keys::J, CS230::Input::Keys::L },
        ControlScheme{ CS230::Input::Keys::Numpad8, CS230::Input::Keys::Numpad5, CS230::Input::Keys::Numpad4, CS230::Input::Keys::Numpad6 }
    };

    const auto& input = Engine::GetInput();

    for (std::size_t i = 0; i < players.size(); ++i)
    {
        const auto& controls = CONTROL_SCHEMES[i];
        auto&       player   = players[i];

        if (input.KeyDown(controls.turnLeft))
            player.rotation += delta_time * 2.0;
        if (input.KeyDown(controls.turnRight))
            player.rotation -= delta_time * 2.0;

        Math::vec2 forward{ std::cos(player.rotation), std::sin(player.rotation) };
        if (input.KeyDown(controls.forward))
            player.position += forward * (player.speed * delta_time);
        if (input.KeyDown(controls.backward))
            player.position -= forward * (player.speed * delta_time);

        // Keep players within the world bounds
        player.position.x = std::clamp(player.position.x, 40.0, worldSize.x - 40.0);
        player.position.y = std::clamp(player.position.y, 40.0, worldSize.y - 40.0);

        updateCameraState(i, delta_time);
    }
}

void DemoCameras::updateCameraState(std::size_t index, double delta_time)
{
    auto& camera = cameras[index];
    const auto& player = players[index];

    const Math::vec2 target_position = player.position;
    const double     target_rotation = (camera.mode == CameraMode::FirstPerson) ? player.rotation : 0.0;

    ease_to_target(camera.position.x, target_position.x, delta_time, 5.0);
    ease_to_target(camera.position.y, target_position.y, delta_time, 5.0);
    ease_to_target(camera.rotation, target_rotation, delta_time, 6.5);
    ease_to_target(camera.zoom, camera.targetZoom, delta_time, 4.0);
}

void DemoCameras::buildViewportLayout(Math::ivec2 window_size)
{
    viewports.clear();
    viewports.reserve(static_cast<std::size_t>(viewportCount));

    switch (viewportCount)
    {
        case 1:
            viewports.push_back({ { 0, 0 }, window_size });
            break;
        case 2:
        {
            const int half_width = window_size.x / 2;
            viewports.push_back({ { 0, 0 }, { half_width, window_size.y } });
            viewports.push_back({ { half_width, 0 }, { window_size.x - half_width, window_size.y } });
            break;
        }
        case 3:
        {
            const int big_width   = static_cast<int>(window_size.x * 0.66);
            const int small_width = window_size.x - big_width;
            const int half_height = window_size.y / 2;
            viewports.push_back({ { 0, 0 }, { big_width, window_size.y } });
            viewports.push_back({ { big_width, half_height }, { small_width, window_size.y - half_height } });
            viewports.push_back({ { big_width, 0 }, { small_width, half_height } });
            break;
        }
        case 4:
        default:
        {
            const int half_width  = window_size.x / 2;
            const int half_height = window_size.y / 2;
            viewports.push_back({ { 0, half_height }, { half_width, window_size.y - half_height } });
            viewports.push_back({ { half_width, half_height }, { window_size.x - half_width, window_size.y - half_height } });
            viewports.push_back({ { 0, 0 }, { half_width, half_height } });
            viewports.push_back({ { half_width, 0 }, { window_size.x - half_width, half_height } });
            break;
        }
    }
}

Math::TransformationMatrix DemoCameras::buildViewProjection(const Camera& camera, Math::ivec2 viewport_size) const
{
    const Math::TranslationMatrix to_origin(Math::vec2{ -camera.position.x, -camera.position.y });
    const Math::RotationMatrix    rotate(-camera.rotation);
    const Math::ScaleMatrix       zoom(camera.zoom);
    const Math::TranslationMatrix to_center(Math::vec2{ viewport_size.x * 0.5, viewport_size.y * 0.5 });

    const auto screen_from_world = to_center * zoom * rotate * to_origin;
    return CS200::build_ndc_matrix(viewport_size) * screen_from_world;
}

void DemoCameras::drawWorld(const Math::TransformationMatrix& view_projection) const
{
    auto& renderer = Engine::GetRenderer2D();

    // Base ground
    renderer.DrawRectangle(Math::TranslationMatrix(worldSize * 0.5) * Math::ScaleMatrix(worldSize), 0x1C2636FF, 0x000000FF, 12.0);

    // Decorative rings to show scale
    renderer.DrawCircle(Math::TranslationMatrix(worldSize * 0.5) * Math::ScaleMatrix({ 600.0, 600.0 }), 0x1C263600, 0x294057FF, 10.0);
    renderer.DrawCircle(Math::TranslationMatrix(worldSize * 0.5) * Math::ScaleMatrix({ 900.0, 900.0 }), 0x1C263600, 0x294057FF, 10.0);

    // Primitive driven shapes
    primitiveRenderer.Draw(GL_POINTS, pointField, view_projection, 5.0f);

    GL::LineWidth(3.0f);
    primitiveRenderer.Draw(GL_LINES, boundaryLines, view_projection);
    GL::LineWidth(4.0f);
    primitiveRenderer.Draw(GL_LINE_STRIP, pathStrip, view_projection);
    GL::LineWidth(5.0f);
    primitiveRenderer.Draw(GL_LINE_LOOP, lakeLoop, view_projection);
    GL::LineWidth(1.0f);

    primitiveRenderer.Draw(GL_TRIANGLES, mountainTriangles, view_projection);
    primitiveRenderer.Draw(GL_TRIANGLE_FAN, sunFan, view_projection);
    primitiveRenderer.Draw(GL_TRIANGLE_STRIP, riverStrip, view_projection);

    if (showGrid)
    {
        const double spacing = 200.0;
        const CS200::RGBA grid_color = 0x1F2A3CFF;
        for (double x = spacing; x < worldSize.x; x += spacing)
        {
            renderer.DrawLine({ x, 0.0 }, { x, worldSize.y }, grid_color, 2.0);
        }
        for (double y = spacing; y < worldSize.y; y += spacing)
        {
            renderer.DrawLine({ 0.0, y }, { worldSize.x, y }, grid_color, 2.0);
        }
    }
}

void DemoCameras::drawPlayers(const Math::TransformationMatrix& view_projection) const
{
    (void)view_projection;
    auto& renderer = Engine::GetRenderer2D();
    for (std::size_t i = 0; i < static_cast<std::size_t>(viewportCount); ++i)
    {
        const auto& player = players[i];

        const auto body_transform = Math::TranslationMatrix(player.position) * Math::RotationMatrix(player.rotation) * Math::ScaleMatrix({ 60.0, 36.0 });
        renderer.DrawRectangle(body_transform, player.color, 0x000000FF, 3.0);

        const auto nose_transform = Math::TranslationMatrix(player.position + Math::vec2{ std::cos(player.rotation) * 40.0, std::sin(player.rotation) * 40.0 }) *
                                    Math::RotationMatrix(player.rotation) * Math::ScaleMatrix({ 24.0, 24.0 });
        renderer.DrawCircle(nose_transform, 0xFFFFFFFF, 0x000000FF, 3.0);
    }
}

void DemoCameras::drawSeparators() const
{
    auto& renderer    = Engine::GetRenderer2D();
    const auto window = Engine::GetWindow().GetSize();
    const double thickness = 10.0;
    const CS200::RGBA line_color = 0x0B0D15FF;

    const auto vertical_bar = [&](double x)
    {
        renderer.DrawRectangle(Math::TranslationMatrix(Math::vec2{ x, window.y * 0.5 }) * Math::ScaleMatrix({ thickness, static_cast<double>(window.y) }), line_color, CS200::CLEAR, 0.0);
    };
    const auto horizontal_bar = [&](double y, double start_x, double end_x)
    {
        const double width = end_x - start_x;
        renderer.DrawRectangle(Math::TranslationMatrix(Math::vec2{ start_x + width * 0.5, y }) * Math::ScaleMatrix({ width, thickness }), line_color, CS200::CLEAR, 0.0);
    };

    switch (viewportCount)
    {
        case 2:
            vertical_bar(window.x * 0.5);
            break;
        case 3:
        {
            const double split_x     = window.x * 0.66;
            const double right_width = window.x - split_x;
            vertical_bar(split_x);
            horizontal_bar(window.y * 0.5, split_x, split_x + right_width);
            break;
        }
        case 4:
        {
            vertical_bar(window.x * 0.5);
            horizontal_bar(window.y * 0.5, 0.0, window.x);
            break;
        }
        default: break;
    }

    // Highlight active viewport corner
    const auto& active_vp = viewports[static_cast<std::size_t>(activeViewport)];
    renderer.DrawRectangle(Math::TranslationMatrix(Math::vec2{ active_vp.origin.x + 32.0, active_vp.origin.y + active_vp.size.y - 32.0 }) * Math::ScaleMatrix({ 48.0, 48.0 }),
                           0xFFCC00AA, CS200::CLEAR, 0.0);
}

void DemoCameras::setupWorldGeometry()
{
    pointField.clear();
    for (int i = 0; i < 90; ++i)
    {
        const float x = static_cast<float>(util::random(0.0, worldSize.x));
        const float y = static_cast<float>(util::random(worldSize.y * 0.5, worldSize.y));
        pointField.push_back({ { x, y }, { 1.0f, 1.0f, 1.0f, 1.0f } });
    }

    const auto deep_blue = rgba_to_float4(0x3FA7D6FF);
    boundaryLines = {
        { { 0.0f, 0.0f }, deep_blue }, { { static_cast<float>(worldSize.x), 0.0f }, deep_blue },
        { { static_cast<float>(worldSize.x), 0.0f }, deep_blue }, { { static_cast<float>(worldSize.x), static_cast<float>(worldSize.y) }, deep_blue },
        { { static_cast<float>(worldSize.x), static_cast<float>(worldSize.y) }, deep_blue }, { { 0.0f, static_cast<float>(worldSize.y) }, deep_blue },
        { { 0.0f, static_cast<float>(worldSize.y) }, deep_blue }, { { 0.0f, 0.0f }, deep_blue }
    };

    pathStrip = {
        { { 200.0f, 200.0f }, rgba_to_float4(0xA7FFEBFF) }, { { 700.0f, 320.0f }, rgba_to_float4(0x81E6D9FF) },
        { { 1100.0f, 280.0f }, rgba_to_float4(0x4FD1C5FF) }, { { 1400.0f, 520.0f }, rgba_to_float4(0x38B2ACFF) },
        { { 1700.0f, 480.0f }, rgba_to_float4(0x2C7A7BFF) }, { { 2000.0f, 680.0f }, rgba_to_float4(0x234E52FF) }
    };

    lakeLoop = {
        { { 1500.0f, 1100.0f }, rgba_to_float4(0x64B5F6FF) },
        { { 1600.0f, 1040.0f }, rgba_to_float4(0x42A5F5FF) },
        { { 1720.0f, 1080.0f }, rgba_to_float4(0x1E88E5FF) },
        { { 1760.0f, 1200.0f }, rgba_to_float4(0x2196F3FF) },
        { { 1650.0f, 1280.0f }, rgba_to_float4(0x64B5F6FF) },
        { { 1540.0f, 1240.0f }, rgba_to_float4(0x90CAF9FF) }
    };

    mountainTriangles = {
        { { 200.0f, 600.0f }, rgba_to_float4(0x6C5B7BFF) }, { { 420.0f, 1040.0f }, rgba_to_float4(0xC06C84FF) }, { { 620.0f, 620.0f }, rgba_to_float4(0xF8B195FF) },
        { { 520.0f, 520.0f }, rgba_to_float4(0x355070FF) }, { { 780.0f, 980.0f }, rgba_to_float4(0x6D597AFF) }, { { 960.0f, 540.0f }, rgba_to_float4(0xB56576FF) }
    };

    // Sun made with a triangle fan
    sunFan.clear();
    sunFan.push_back({ { 1200.0f, 1500.0f }, rgba_to_float4(0xFFDD55FF) });
    const float radius = 120.0f;
    for (int i = 0; i <= 16; ++i)
    {
        const float angle = static_cast<float>(i) / 16.0f * 2.0f * static_cast<float>(std::numbers::pi);
        sunFan.push_back({ { 1200.0f + std::cos(angle) * radius, 1500.0f + std::sin(angle) * radius }, rgba_to_float4(0xFF9F1CFF) });
    }

    // Gentle river using triangle strip
    riverStrip.clear();
    const float river_start_x = 300.0f;
    const float river_end_x   = 2200.0f;
    const float river_width   = 90.0f;
    for (int i = 0; i <= 12; ++i)
    {
        const float t      = static_cast<float>(i) / 12.0f;
        const float x      = river_start_x + (river_end_x - river_start_x) * t;
        const float offset = std::sin(t * 3.1415f * 2.5f) * 120.0f;
        const float y      = 700.0f + offset;
        const auto  color  = rgba_to_float4(0x4FC3F7FF);
        riverStrip.push_back({ { x, y - river_width }, color });
        riverStrip.push_back({ { x, y + river_width }, color });
    }
}
