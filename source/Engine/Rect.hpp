/**
 * \file
 * \author Rudy Castan
 * \author Jonathan Holmes
 * \author Junseok Lee 
 * \date 2025 Fall
 * \par CS200 Computer Graphics I
 * \copyright DigiPen Institute of Technology
 */

#pragma once
#include "Vec2.hpp"
#include <algorithm>

namespace Math {
    struct [[nodiscard]] rect {
        Math::vec2 point_1{ 0.0, 0.0 };
        Math::vec2 point_2{ 0.0, 0.0 };
        double Left() const noexcept;
        double Right()const noexcept;
        double Top()const noexcept;
        double Bottom()const noexcept;

        Math::vec2 Size() const noexcept {
            return {
                Right() - Left(),
                Top() - Bottom()
            };
        }
    };
    struct [[nodiscard]] irect {
        Math::ivec2 point_1{ 0, 0 };
        Math::ivec2 point_2{ 0, 0 };

        int Left()const noexcept;
        int Right()const noexcept;
        int Top()const noexcept;
        int Bottom()const noexcept;

        Math::ivec2 Size() const noexcept {
            return {
                Right() - Left(),
                Top() - Bottom()
            };
        }
    };
}