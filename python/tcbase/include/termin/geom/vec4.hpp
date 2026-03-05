#pragma once

#include <cmath>

namespace termin {


// ============================================================================
// Vec4 (double)
// ============================================================================

struct Vec4 {
    double x, y, z, w;

    Vec4() : x(0), y(0), z(0), w(0) {}
    Vec4(double x, double y, double z, double w) : x(x), y(y), z(z), w(w) {}

    double& operator[](int i) { return (&x)[i]; }
    double operator[](int i) const { return (&x)[i]; }

    Vec4 operator+(const Vec4& v) const { return {x + v.x, y + v.y, z + v.z, w + v.w}; }
    Vec4 operator-(const Vec4& v) const { return {x - v.x, y - v.y, z - v.z, w - v.w}; }
    Vec4 operator*(double s) const { return {x * s, y * s, z * s, w * s}; }
    Vec4 operator/(double s) const { return {x / s, y / s, z / s, w / s}; }
    Vec4 operator-() const { return {-x, -y, -z, -w}; }

    Vec4& operator+=(const Vec4& v) { x += v.x; y += v.y; z += v.z; w += v.w; return *this; }
    Vec4& operator-=(const Vec4& v) { x -= v.x; y -= v.y; z -= v.z; w -= v.w; return *this; }
    Vec4& operator*=(double s) { x *= s; y *= s; z *= s; w *= s; return *this; }
    Vec4& operator/=(double s) { x /= s; y /= s; z /= s; w /= s; return *this; }

    bool operator==(const Vec4& v) const { return x == v.x && y == v.y && z == v.z && w == v.w; }
    bool operator!=(const Vec4& v) const { return !(*this == v); }

    double dot(const Vec4& v) const { return x * v.x + y * v.y + z * v.z + w * v.w; }

    double norm() const { return std::sqrt(x * x + y * y + z * z + w * w); }
    double norm_squared() const { return x * x + y * y + z * z + w * w; }

    Vec4 normalized() const {
        double n = norm();
        return n > 1e-10 ? *this / n : Vec4{0, 0, 0, 1};
    }

    static Vec4 zero() { return {0, 0, 0, 0}; }
    static Vec4 unit_x() { return {1, 0, 0, 0}; }
    static Vec4 unit_y() { return {0, 1, 0, 0}; }
    static Vec4 unit_z() { return {0, 0, 1, 0}; }
    static Vec4 unit_w() { return {0, 0, 0, 1}; }
};

inline Vec4 operator*(double s, const Vec4& v) { return v * s; }


// ============================================================================
// Vec4f (float)
// ============================================================================

struct Vec4f {
    float x, y, z, w;

    Vec4f() : x(0), y(0), z(0), w(0) {}
    Vec4f(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    explicit Vec4f(const Vec4& v) : x(static_cast<float>(v.x)), y(static_cast<float>(v.y)), z(static_cast<float>(v.z)), w(static_cast<float>(v.w)) {}

    float& operator[](int i) { return (&x)[i]; }
    float operator[](int i) const { return (&x)[i]; }

    Vec4f operator+(const Vec4f& v) const { return {x + v.x, y + v.y, z + v.z, w + v.w}; }
    Vec4f operator-(const Vec4f& v) const { return {x - v.x, y - v.y, z - v.z, w - v.w}; }
    Vec4f operator*(float s) const { return {x * s, y * s, z * s, w * s}; }
    Vec4f operator/(float s) const { return {x / s, y / s, z / s, w / s}; }
    Vec4f operator-() const { return {-x, -y, -z, -w}; }

    Vec4f& operator+=(const Vec4f& v) { x += v.x; y += v.y; z += v.z; w += v.w; return *this; }
    Vec4f& operator-=(const Vec4f& v) { x -= v.x; y -= v.y; z -= v.z; w -= v.w; return *this; }
    Vec4f& operator*=(float s) { x *= s; y *= s; z *= s; w *= s; return *this; }
    Vec4f& operator/=(float s) { x /= s; y /= s; z /= s; w /= s; return *this; }

    bool operator==(const Vec4f& v) const { return x == v.x && y == v.y && z == v.z && w == v.w; }
    bool operator!=(const Vec4f& v) const { return !(*this == v); }

    float dot(const Vec4f& v) const { return x * v.x + y * v.y + z * v.z + w * v.w; }

    float norm() const { return std::sqrt(x * x + y * y + z * z + w * w); }
    float norm_squared() const { return x * x + y * y + z * z + w * w; }

    Vec4f normalized() const {
        float n = norm();
        return n > 1e-6f ? *this / n : Vec4f{0, 0, 0, 1};
    }

    Vec4 to_double() const { return {x, y, z, w}; }

    static Vec4f zero() { return {0, 0, 0, 0}; }
    static Vec4f unit_x() { return {1, 0, 0, 0}; }
    static Vec4f unit_y() { return {0, 1, 0, 0}; }
    static Vec4f unit_z() { return {0, 0, 1, 0}; }
    static Vec4f unit_w() { return {0, 0, 0, 1}; }
};

inline Vec4f operator*(float s, const Vec4f& v) { return v * s; }


// ============================================================================
// Vec4i (int)
// ============================================================================

struct Vec4i {
    int x, y, z, w;

    Vec4i() : x(0), y(0), z(0), w(0) {}
    Vec4i(int x, int y, int z, int w) : x(x), y(y), z(z), w(w) {}

    int& operator[](int i) { return (&x)[i]; }
    int operator[](int i) const { return (&x)[i]; }

    Vec4i operator+(const Vec4i& v) const { return {x + v.x, y + v.y, z + v.z, w + v.w}; }
    Vec4i operator-(const Vec4i& v) const { return {x - v.x, y - v.y, z - v.z, w - v.w}; }
    Vec4i operator*(int s) const { return {x * s, y * s, z * s, w * s}; }
    Vec4i operator/(int s) const { return {x / s, y / s, z / s, w / s}; }
    Vec4i operator-() const { return {-x, -y, -z, -w}; }

    Vec4i& operator+=(const Vec4i& v) { x += v.x; y += v.y; z += v.z; w += v.w; return *this; }
    Vec4i& operator-=(const Vec4i& v) { x -= v.x; y -= v.y; z -= v.z; w -= v.w; return *this; }
    Vec4i& operator*=(int s) { x *= s; y *= s; z *= s; w *= s; return *this; }
    Vec4i& operator/=(int s) { x /= s; y /= s; z /= s; w /= s; return *this; }

    bool operator==(const Vec4i& v) const { return x == v.x && y == v.y && z == v.z && w == v.w; }
    bool operator!=(const Vec4i& v) const { return !(*this == v); }

    int dot(const Vec4i& v) const { return x * v.x + y * v.y + z * v.z + w * v.w; }

    Vec4 to_double() const { return {static_cast<double>(x), static_cast<double>(y), static_cast<double>(z), static_cast<double>(w)}; }
    Vec4f to_float() const { return {static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), static_cast<float>(w)}; }

    static Vec4i zero() { return {0, 0, 0, 0}; }
    static Vec4i unit_x() { return {1, 0, 0, 0}; }
    static Vec4i unit_y() { return {0, 1, 0, 0}; }
    static Vec4i unit_z() { return {0, 0, 1, 0}; }
    static Vec4i unit_w() { return {0, 0, 0, 1}; }
};

inline Vec4i operator*(int s, const Vec4i& v) { return v * s; }


} // namespace termin
