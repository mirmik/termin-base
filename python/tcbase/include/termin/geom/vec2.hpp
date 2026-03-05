#pragma once

#include <cmath>

namespace termin {


// ============================================================================
// Vec2 (double)
// ============================================================================

struct Vec2 {
    double x, y;

    Vec2() : x(0), y(0) {}
    Vec2(double x, double y) : x(x), y(y) {}

    double& operator[](int i) { return (&x)[i]; }
    double operator[](int i) const { return (&x)[i]; }

    Vec2 operator+(const Vec2& v) const { return {x + v.x, y + v.y}; }
    Vec2 operator-(const Vec2& v) const { return {x - v.x, y - v.y}; }
    Vec2 operator*(double s) const { return {x * s, y * s}; }
    Vec2 operator/(double s) const { return {x / s, y / s}; }
    Vec2 operator-() const { return {-x, -y}; }

    Vec2& operator+=(const Vec2& v) { x += v.x; y += v.y; return *this; }
    Vec2& operator-=(const Vec2& v) { x -= v.x; y -= v.y; return *this; }
    Vec2& operator*=(double s) { x *= s; y *= s; return *this; }
    Vec2& operator/=(double s) { x /= s; y /= s; return *this; }

    bool operator==(const Vec2& v) const { return x == v.x && y == v.y; }
    bool operator!=(const Vec2& v) const { return !(*this == v); }

    double dot(const Vec2& v) const { return x * v.x + y * v.y; }
    double cross(const Vec2& v) const { return x * v.y - y * v.x; }

    double norm() const { return std::sqrt(x * x + y * y); }
    double norm_squared() const { return x * x + y * y; }

    Vec2 normalized() const {
        double n = norm();
        return n > 1e-10 ? *this / n : Vec2{1, 0};
    }

    static Vec2 zero() { return {0, 0}; }
    static Vec2 unit_x() { return {1, 0}; }
    static Vec2 unit_y() { return {0, 1}; }
};

inline Vec2 operator*(double s, const Vec2& v) { return v * s; }


// ============================================================================
// Vec2f (float)
// ============================================================================

struct Vec2f {
    float x, y;

    Vec2f() : x(0), y(0) {}
    Vec2f(float x, float y) : x(x), y(y) {}
    explicit Vec2f(const Vec2& v) : x(static_cast<float>(v.x)), y(static_cast<float>(v.y)) {}

    float& operator[](int i) { return (&x)[i]; }
    float operator[](int i) const { return (&x)[i]; }

    Vec2f operator+(const Vec2f& v) const { return {x + v.x, y + v.y}; }
    Vec2f operator-(const Vec2f& v) const { return {x - v.x, y - v.y}; }
    Vec2f operator*(float s) const { return {x * s, y * s}; }
    Vec2f operator/(float s) const { return {x / s, y / s}; }
    Vec2f operator-() const { return {-x, -y}; }

    Vec2f& operator+=(const Vec2f& v) { x += v.x; y += v.y; return *this; }
    Vec2f& operator-=(const Vec2f& v) { x -= v.x; y -= v.y; return *this; }
    Vec2f& operator*=(float s) { x *= s; y *= s; return *this; }
    Vec2f& operator/=(float s) { x /= s; y /= s; return *this; }

    bool operator==(const Vec2f& v) const { return x == v.x && y == v.y; }
    bool operator!=(const Vec2f& v) const { return !(*this == v); }

    float dot(const Vec2f& v) const { return x * v.x + y * v.y; }
    float cross(const Vec2f& v) const { return x * v.y - y * v.x; }

    float norm() const { return std::sqrt(x * x + y * y); }
    float norm_squared() const { return x * x + y * y; }

    Vec2f normalized() const {
        float n = norm();
        return n > 1e-6f ? *this / n : Vec2f{1, 0};
    }

    Vec2 to_double() const { return {x, y}; }

    static Vec2f zero() { return {0, 0}; }
    static Vec2f unit_x() { return {1, 0}; }
    static Vec2f unit_y() { return {0, 1}; }
};

inline Vec2f operator*(float s, const Vec2f& v) { return v * s; }


// ============================================================================
// Vec2i (int)
// ============================================================================

struct Vec2i {
    int x, y;

    Vec2i() : x(0), y(0) {}
    Vec2i(int x, int y) : x(x), y(y) {}

    int& operator[](int i) { return (&x)[i]; }
    int operator[](int i) const { return (&x)[i]; }

    Vec2i operator+(const Vec2i& v) const { return {x + v.x, y + v.y}; }
    Vec2i operator-(const Vec2i& v) const { return {x - v.x, y - v.y}; }
    Vec2i operator*(int s) const { return {x * s, y * s}; }
    Vec2i operator/(int s) const { return {x / s, y / s}; }
    Vec2i operator-() const { return {-x, -y}; }

    Vec2i& operator+=(const Vec2i& v) { x += v.x; y += v.y; return *this; }
    Vec2i& operator-=(const Vec2i& v) { x -= v.x; y -= v.y; return *this; }
    Vec2i& operator*=(int s) { x *= s; y *= s; return *this; }
    Vec2i& operator/=(int s) { x /= s; y /= s; return *this; }

    bool operator==(const Vec2i& v) const { return x == v.x && y == v.y; }
    bool operator!=(const Vec2i& v) const { return !(*this == v); }

    int dot(const Vec2i& v) const { return x * v.x + y * v.y; }
    int cross(const Vec2i& v) const { return x * v.y - y * v.x; }

    Vec2 to_double() const { return {static_cast<double>(x), static_cast<double>(y)}; }
    Vec2f to_float() const { return {static_cast<float>(x), static_cast<float>(y)}; }

    static Vec2i zero() { return {0, 0}; }
    static Vec2i unit_x() { return {1, 0}; }
    static Vec2i unit_y() { return {0, 1}; }
};

inline Vec2i operator*(int s, const Vec2i& v) { return v * s; }


} // namespace termin
