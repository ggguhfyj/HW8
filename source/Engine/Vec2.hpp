/**
 * \file
 * \author Jonathan Holmes
 * \author JUNSEOK LEE
 * \date 2025 Fall
 * \par CS200 Computer Graphics I
 * \copyright DigiPen Institute of Technology
 */


#pragma once
#include <iostream>
#include <cmath>
#include <limits>

namespace Math
{
    struct vec2;

    struct ivec2
    {
        int x{0};
        int y{0};

        constexpr ivec2() noexcept = default;
        constexpr ivec2(int x_, int y_) noexcept : x(x_), y(y_) {}
        explicit constexpr ivec2(int xy) noexcept : x(xy), y(xy) {}
        explicit ivec2(const vec2& v) noexcept;

        constexpr ivec2& operator+=(const ivec2& rhs) noexcept { x+=rhs.x; y+=rhs.y; return *this; }
        constexpr ivec2& operator-=(const ivec2& rhs) noexcept { x-=rhs.x; y-=rhs.y; return *this; }
        constexpr ivec2& operator*=(int scalar) noexcept { x*=scalar; y*=scalar; return *this; }
        constexpr ivec2& operator/=(int scalar) noexcept { x/=scalar; y/=scalar; return *this; }

        constexpr ivec2 operator+() const noexcept { return *this; }
        constexpr ivec2 operator-() const noexcept { return {-x, -y}; }
    };

    constexpr ivec2 operator+(const ivec2& lhs, const ivec2& rhs) noexcept { return {lhs.x+rhs.x, lhs.y+rhs.y}; }
    constexpr ivec2 operator-(const ivec2& lhs, const ivec2& rhs) noexcept { return {lhs.x-rhs.x, lhs.y-rhs.y}; }
    constexpr ivec2 operator*(const ivec2& v, int s) noexcept { return {v.x*s, v.y*s}; }
    constexpr ivec2 operator*(int s, const ivec2& v) noexcept { return v*s; }
    constexpr ivec2 operator/(const ivec2& v, int s) noexcept { return {v.x/s, v.y/s}; }

    constexpr bool operator==(const ivec2& lhs, const ivec2& rhs) noexcept { return lhs.x==rhs.x && lhs.y==rhs.y; }
    constexpr bool operator!=(const ivec2& lhs, const ivec2& rhs) noexcept { return !(lhs==rhs); }

    std::ostream& operator<<(std::ostream& os, const ivec2& v);
    std::istream& operator>>(std::istream& is, ivec2& v);

    struct vec2
    {
        double x{0.0};
        double y{0.0};

        constexpr vec2() noexcept = default;
        constexpr vec2(double x_, double y_) noexcept : x(x_), y(y_) {}
        explicit constexpr vec2(double xy) noexcept : x(xy), y(xy) {}
        explicit vec2(const ivec2& v) noexcept : x(v.x), y(v.y) {}

        Math::vec2 Normalize() noexcept;


        double Length() const ;
        constexpr vec2 operator+() const noexcept { return *this; }
        constexpr vec2 operator-() const noexcept { return {-x, -y}; }

        constexpr vec2& operator+=(const vec2& rhs) noexcept { x+=rhs.x; y+=rhs.y; return *this; }
        constexpr vec2& operator-=(const vec2& rhs) noexcept { x-=rhs.x; y-=rhs.y; return *this; }
        constexpr vec2& operator*=(double s) noexcept { x*=s; y*=s; return *this; }
        constexpr vec2& operator/=(double s) noexcept { x/=s; y/=s; return *this; }

    };

    constexpr vec2 operator+(const vec2& lhs, const vec2& rhs) noexcept { return {lhs.x+rhs.x, lhs.y+rhs.y}; }
    constexpr vec2 operator-(const vec2& lhs, const vec2& rhs) noexcept { return {lhs.x-rhs.x, lhs.y-rhs.y}; }
    constexpr vec2 operator*(const vec2& v, double s) noexcept { return {v.x*s, v.y*s}; }
    constexpr vec2 operator*(double s, const vec2& v) noexcept { return v*s; }
    constexpr vec2 operator/(const vec2& v, double s) noexcept { return {v.x/s, v.y/s}; }

    std::ostream& operator<<(std::ostream& os, const vec2& v);
    std::istream& operator>>(std::istream& is, vec2& v);
}
