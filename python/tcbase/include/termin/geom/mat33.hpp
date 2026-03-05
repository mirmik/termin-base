#pragma once

#include "vec3.hpp"
#include <cmath>
#include <cstring>

namespace termin {


// ============================================================================
// Mat33f (float) - 3x3 Matrix in column-major order
// ============================================================================

struct Mat33f {
    float data[9];  // Column-major: [col0, col1, col2]

    Mat33f() { std::memset(data, 0, sizeof(data)); }

    // Access by column and row: m(col, row)
    float& operator()(int col, int row) { return data[col * 3 + row]; }
    float operator()(int col, int row) const { return data[col * 3 + row]; }

    float* ptr() { return data; }
    const float* ptr() const { return data; }

    static Mat33f identity() {
        Mat33f m;
        m(0, 0) = 1; m(1, 1) = 1; m(2, 2) = 1;
        return m;
    }

    static Mat33f zero() {
        return Mat33f();
    }

    // Matrix multiplication
    Mat33f operator*(const Mat33f& b) const {
        Mat33f result;
        for (int col = 0; col < 3; ++col) {
            for (int row = 0; row < 3; ++row) {
                float sum = 0;
                for (int k = 0; k < 3; ++k) {
                    sum += (*this)(k, row) * b(col, k);
                }
                result(col, row) = sum;
            }
        }
        return result;
    }

    // Transform vector
    Vec3f transform(const Vec3f& v) const {
        return {
            (*this)(0, 0) * v.x + (*this)(1, 0) * v.y + (*this)(2, 0) * v.z,
            (*this)(0, 1) * v.x + (*this)(1, 1) * v.y + (*this)(2, 1) * v.z,
            (*this)(0, 2) * v.x + (*this)(1, 2) * v.y + (*this)(2, 2) * v.z
        };
    }

    Vec3 transform(const Vec3& v) const {
        return {
            (*this)(0, 0) * v.x + (*this)(1, 0) * v.y + (*this)(2, 0) * v.z,
            (*this)(0, 1) * v.x + (*this)(1, 1) * v.y + (*this)(2, 1) * v.z,
            (*this)(0, 2) * v.x + (*this)(1, 2) * v.y + (*this)(2, 2) * v.z
        };
    }

    // Transpose
    Mat33f transposed() const {
        Mat33f result;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                result(i, j) = (*this)(j, i);
            }
        }
        return result;
    }

    // Determinant
    float determinant() const {
        return (*this)(0, 0) * ((*this)(1, 1) * (*this)(2, 2) - (*this)(2, 1) * (*this)(1, 2))
             - (*this)(1, 0) * ((*this)(0, 1) * (*this)(2, 2) - (*this)(2, 1) * (*this)(0, 2))
             + (*this)(2, 0) * ((*this)(0, 1) * (*this)(1, 2) - (*this)(1, 1) * (*this)(0, 2));
    }

    // Inverse
    Mat33f inverse() const {
        float det = determinant();
        if (std::abs(det) < 1e-6f) {
            return identity();
        }

        float inv_det = 1.0f / det;
        Mat33f inv;

        inv(0, 0) = ((*this)(1, 1) * (*this)(2, 2) - (*this)(2, 1) * (*this)(1, 2)) * inv_det;
        inv(0, 1) = ((*this)(0, 2) * (*this)(2, 1) - (*this)(0, 1) * (*this)(2, 2)) * inv_det;
        inv(0, 2) = ((*this)(0, 1) * (*this)(1, 2) - (*this)(0, 2) * (*this)(1, 1)) * inv_det;

        inv(1, 0) = ((*this)(1, 2) * (*this)(2, 0) - (*this)(1, 0) * (*this)(2, 2)) * inv_det;
        inv(1, 1) = ((*this)(0, 0) * (*this)(2, 2) - (*this)(0, 2) * (*this)(2, 0)) * inv_det;
        inv(1, 2) = ((*this)(1, 0) * (*this)(0, 2) - (*this)(0, 0) * (*this)(1, 2)) * inv_det;

        inv(2, 0) = ((*this)(1, 0) * (*this)(2, 1) - (*this)(2, 0) * (*this)(1, 1)) * inv_det;
        inv(2, 1) = ((*this)(2, 0) * (*this)(0, 1) - (*this)(0, 0) * (*this)(2, 1)) * inv_det;
        inv(2, 2) = ((*this)(0, 0) * (*this)(1, 1) - (*this)(1, 0) * (*this)(0, 1)) * inv_det;

        return inv;
    }

    // Scale matrix
    static Mat33f scale(float s) {
        Mat33f m;
        m(0, 0) = s; m(1, 1) = s; m(2, 2) = s;
        return m;
    }

    static Mat33f scale(const Vec3f& s) {
        Mat33f m;
        m(0, 0) = s.x; m(1, 1) = s.y; m(2, 2) = s.z;
        return m;
    }

    // Rotation around axis (angle in radians)
    static Mat33f rotation_x(float angle) {
        float c = std::cos(angle), s = std::sin(angle);
        Mat33f m = identity();
        m(1, 1) = c;  m(2, 1) = -s;
        m(1, 2) = s;  m(2, 2) = c;
        return m;
    }

    static Mat33f rotation_y(float angle) {
        float c = std::cos(angle), s = std::sin(angle);
        Mat33f m = identity();
        m(0, 0) = c;  m(2, 0) = s;
        m(0, 2) = -s; m(2, 2) = c;
        return m;
    }

    static Mat33f rotation_z(float angle) {
        float c = std::cos(angle), s = std::sin(angle);
        Mat33f m = identity();
        m(0, 0) = c;  m(1, 0) = -s;
        m(0, 1) = s;  m(1, 1) = c;
        return m;
    }

    static Mat33f rotation_axis_angle(const Vec3f& axis, float angle) {
        Vec3f a = axis.normalized();
        float c = std::cos(angle), s = std::sin(angle);
        float t = 1.0f - c;

        Mat33f m;
        m(0, 0) = t * a.x * a.x + c;
        m(0, 1) = t * a.x * a.y + s * a.z;
        m(0, 2) = t * a.x * a.z - s * a.y;

        m(1, 0) = t * a.x * a.y - s * a.z;
        m(1, 1) = t * a.y * a.y + c;
        m(1, 2) = t * a.y * a.z + s * a.x;

        m(2, 0) = t * a.x * a.z + s * a.y;
        m(2, 1) = t * a.y * a.z - s * a.x;
        m(2, 2) = t * a.z * a.z + c;

        return m;
    }
};


// ============================================================================
// Mat33 (double) - 3x3 Matrix in column-major order
// ============================================================================

struct Mat33 {
    double data[9];  // Column-major: [col0, col1, col2]

    Mat33() { std::memset(data, 0, sizeof(data)); }

    // Access by column and row: m(col, row)
    double& operator()(int col, int row) { return data[col * 3 + row]; }
    double operator()(int col, int row) const { return data[col * 3 + row]; }

    double* ptr() { return data; }
    const double* ptr() const { return data; }

    static Mat33 identity() {
        Mat33 m;
        m(0, 0) = 1; m(1, 1) = 1; m(2, 2) = 1;
        return m;
    }

    static Mat33 zero() {
        return Mat33();
    }

    // Matrix multiplication
    Mat33 operator*(const Mat33& b) const {
        Mat33 result;
        for (int col = 0; col < 3; ++col) {
            for (int row = 0; row < 3; ++row) {
                double sum = 0;
                for (int k = 0; k < 3; ++k) {
                    sum += (*this)(k, row) * b(col, k);
                }
                result(col, row) = sum;
            }
        }
        return result;
    }

    // Transform vector
    Vec3 transform(const Vec3& v) const {
        return {
            (*this)(0, 0) * v.x + (*this)(1, 0) * v.y + (*this)(2, 0) * v.z,
            (*this)(0, 1) * v.x + (*this)(1, 1) * v.y + (*this)(2, 1) * v.z,
            (*this)(0, 2) * v.x + (*this)(1, 2) * v.y + (*this)(2, 2) * v.z
        };
    }

    // Transpose
    Mat33 transposed() const {
        Mat33 result;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                result(i, j) = (*this)(j, i);
            }
        }
        return result;
    }

    // Determinant
    double determinant() const {
        return (*this)(0, 0) * ((*this)(1, 1) * (*this)(2, 2) - (*this)(2, 1) * (*this)(1, 2))
             - (*this)(1, 0) * ((*this)(0, 1) * (*this)(2, 2) - (*this)(2, 1) * (*this)(0, 2))
             + (*this)(2, 0) * ((*this)(0, 1) * (*this)(1, 2) - (*this)(1, 1) * (*this)(0, 2));
    }

    // Inverse
    Mat33 inverse() const {
        double det = determinant();
        if (std::abs(det) < 1e-10) {
            return identity();
        }

        double inv_det = 1.0 / det;
        Mat33 inv;

        inv(0, 0) = ((*this)(1, 1) * (*this)(2, 2) - (*this)(2, 1) * (*this)(1, 2)) * inv_det;
        inv(0, 1) = ((*this)(0, 2) * (*this)(2, 1) - (*this)(0, 1) * (*this)(2, 2)) * inv_det;
        inv(0, 2) = ((*this)(0, 1) * (*this)(1, 2) - (*this)(0, 2) * (*this)(1, 1)) * inv_det;

        inv(1, 0) = ((*this)(1, 2) * (*this)(2, 0) - (*this)(1, 0) * (*this)(2, 2)) * inv_det;
        inv(1, 1) = ((*this)(0, 0) * (*this)(2, 2) - (*this)(0, 2) * (*this)(2, 0)) * inv_det;
        inv(1, 2) = ((*this)(1, 0) * (*this)(0, 2) - (*this)(0, 0) * (*this)(1, 2)) * inv_det;

        inv(2, 0) = ((*this)(1, 0) * (*this)(2, 1) - (*this)(2, 0) * (*this)(1, 1)) * inv_det;
        inv(2, 1) = ((*this)(2, 0) * (*this)(0, 1) - (*this)(0, 0) * (*this)(2, 1)) * inv_det;
        inv(2, 2) = ((*this)(0, 0) * (*this)(1, 1) - (*this)(1, 0) * (*this)(0, 1)) * inv_det;

        return inv;
    }

    // Conversion
    Mat33f to_float() const {
        Mat33f m;
        for (int i = 0; i < 9; ++i) {
            m.data[i] = static_cast<float>(data[i]);
        }
        return m;
    }

    // Scale matrix
    static Mat33 scale(double s) {
        Mat33 m;
        m(0, 0) = s; m(1, 1) = s; m(2, 2) = s;
        return m;
    }

    static Mat33 scale(const Vec3& s) {
        Mat33 m;
        m(0, 0) = s.x; m(1, 1) = s.y; m(2, 2) = s.z;
        return m;
    }

    // Rotation around axis (angle in radians)
    static Mat33 rotation_x(double angle) {
        double c = std::cos(angle), s = std::sin(angle);
        Mat33 m = identity();
        m(1, 1) = c;  m(2, 1) = -s;
        m(1, 2) = s;  m(2, 2) = c;
        return m;
    }

    static Mat33 rotation_y(double angle) {
        double c = std::cos(angle), s = std::sin(angle);
        Mat33 m = identity();
        m(0, 0) = c;  m(2, 0) = s;
        m(0, 2) = -s; m(2, 2) = c;
        return m;
    }

    static Mat33 rotation_z(double angle) {
        double c = std::cos(angle), s = std::sin(angle);
        Mat33 m = identity();
        m(0, 0) = c;  m(1, 0) = -s;
        m(0, 1) = s;  m(1, 1) = c;
        return m;
    }

    static Mat33 rotation_axis_angle(const Vec3& axis, double angle) {
        Vec3 a = axis.normalized();
        double c = std::cos(angle), s = std::sin(angle);
        double t = 1.0 - c;

        Mat33 m;
        m(0, 0) = t * a.x * a.x + c;
        m(0, 1) = t * a.x * a.y + s * a.z;
        m(0, 2) = t * a.x * a.z - s * a.y;

        m(1, 0) = t * a.x * a.y - s * a.z;
        m(1, 1) = t * a.y * a.y + c;
        m(1, 2) = t * a.y * a.z + s * a.x;

        m(2, 0) = t * a.x * a.z + s * a.y;
        m(2, 1) = t * a.y * a.z - s * a.x;
        m(2, 2) = t * a.z * a.z + c;

        return m;
    }
};


} // namespace termin
