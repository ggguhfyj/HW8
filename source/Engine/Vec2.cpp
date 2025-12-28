/**
 * \file
 * \author Jonathan Holmes
 * \author JUNSEOK LEE
 * \date 2025 Fall
 * \par CS200 Computer Graphics I
 * \copyright DigiPen Institute of Technology
 */

#include "Vec2.hpp"
#include <cmath>
#include <limits>

namespace Math
{
    ivec2::ivec2(const vec2& v) noexcept
        : x(static_cast<int>(v.x)), y(static_cast<int>(v.y)) {}

    vec2 vec2::Normalize() noexcept
    {
        double length_sq = x * x + y * y;
        if (length_sq > 0.0)
        {
            double length = std::sqrt(length_sq);
            x /= length;
            y /= length;
        }
        return vec2{x,y};
    }

    double vec2::Length() const
    {
        return sqrt((x*x) + (y*y)); 
    }

    std::ostream& operator<<(std::ostream& os, const ivec2& v)
    {
        os << "{ " << v.x << ", " << v.y << " }";
        return os;
    }

    std::istream& operator>>(std::istream& is, ivec2& v)
    {
        is >> v.x >> v.y;
        return is;
    }

    std::ostream& operator<<(std::ostream& os, const vec2& v)
    {
        os << "{ " << v.x << ", " << v.y << " }";
        return os;
    }

    std::istream& operator>>(std::istream& is, vec2& v)
    {
        is >> v.x >> v.y;
        return is;
    }
}
