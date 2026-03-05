#pragma once

#include "vec3.hpp"
#include "quat.hpp"
#include <cmath>
#include <cstring>

namespace termin {


// ============================================================================
// Mat44f (float) - 4x4 Matrix in column-major order (OpenGL convention)
// ============================================================================

/**
 * 4x4 Matrix in column-major order (OpenGL convention).
 *
 * Memory layout: m[col][row] or m[col * 4 + row]
 *
 * Coordinate convention: Y-forward, Z-up
 *   - X: right
 *   - Y: forward (depth, camera looks along +Y)
 *   - Z: up
 */
struct Mat44f {
    float data[16];  // Column-major: [col0, col1, col2, col3]

    Mat44f() { std::memset(data, 0, sizeof(data)); }

    // Access by column and row: m(col, row)
    float& operator()(int col, int row) { return data[col * 4 + row]; }
    float operator()(int col, int row) const { return data[col * 4 + row]; }

    // Raw data access
    float* ptr() { return data; }
    const float* ptr() const { return data; }

    // Identity matrix
    static Mat44f identity() {
        Mat44f m;
        m(0, 0) = 1; m(1, 1) = 1; m(2, 2) = 1; m(3, 3) = 1;
        return m;
    }

    // Zero matrix
    static Mat44f zero() {
        return Mat44f();
    }

    // Matrix multiplication: this * other
    Mat44f operator*(const Mat44f& b) const {
        Mat44f result;
        for (int col = 0; col < 4; ++col) {
            for (int row = 0; row < 4; ++row) {
                float sum = 0;
                for (int k = 0; k < 4; ++k) {
                    sum += (*this)(k, row) * b(col, k);
                }
                result(col, row) = sum;
            }
        }
        return result;
    }

    // Transform point (w=1)
    Vec3 transform_point(const Vec3& p) const {
        float x = (*this)(0, 0) * static_cast<float>(p.x) + (*this)(1, 0) * static_cast<float>(p.y) + (*this)(2, 0) * static_cast<float>(p.z) + (*this)(3, 0);
        float y = (*this)(0, 1) * static_cast<float>(p.x) + (*this)(1, 1) * static_cast<float>(p.y) + (*this)(2, 1) * static_cast<float>(p.z) + (*this)(3, 1);
        float z = (*this)(0, 2) * static_cast<float>(p.x) + (*this)(1, 2) * static_cast<float>(p.y) + (*this)(2, 2) * static_cast<float>(p.z) + (*this)(3, 2);
        float w = (*this)(0, 3) * static_cast<float>(p.x) + (*this)(1, 3) * static_cast<float>(p.y) + (*this)(2, 3) * static_cast<float>(p.z) + (*this)(3, 3);
        if (std::abs(w) > 1e-6f) {
            return {x / w, y / w, z / w};
        }
        return {x, y, z};
    }

    // Transform direction (w=0)
    Vec3 transform_direction(const Vec3& d) const {
        return {
            (*this)(0, 0) * d.x + (*this)(1, 0) * d.y + (*this)(2, 0) * d.z,
            (*this)(0, 1) * d.x + (*this)(1, 1) * d.y + (*this)(2, 1) * d.z,
            (*this)(0, 2) * d.x + (*this)(1, 2) * d.y + (*this)(2, 2) * d.z
        };
    }

    // Transpose
    Mat44f transposed() const {
        Mat44f result;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                result(i, j) = (*this)(j, i);
            }
        }
        return result;
    }

    // Inverse (general 4x4 matrix inverse using cofactors)
    Mat44f inverse() const {
        Mat44f inv;
        const float* m = data;

        inv.data[0] = m[5] * m[10] * m[15] - m[5] * m[11] * m[14] - m[9] * m[6] * m[15] +
                      m[9] * m[7] * m[14] + m[13] * m[6] * m[11] - m[13] * m[7] * m[10];
        inv.data[4] = -m[4] * m[10] * m[15] + m[4] * m[11] * m[14] + m[8] * m[6] * m[15] -
                      m[8] * m[7] * m[14] - m[12] * m[6] * m[11] + m[12] * m[7] * m[10];
        inv.data[8] = m[4] * m[9] * m[15] - m[4] * m[11] * m[13] - m[8] * m[5] * m[15] +
                      m[8] * m[7] * m[13] + m[12] * m[5] * m[11] - m[12] * m[7] * m[9];
        inv.data[12] = -m[4] * m[9] * m[14] + m[4] * m[10] * m[13] + m[8] * m[5] * m[14] -
                       m[8] * m[6] * m[13] - m[12] * m[5] * m[10] + m[12] * m[6] * m[9];
        inv.data[1] = -m[1] * m[10] * m[15] + m[1] * m[11] * m[14] + m[9] * m[2] * m[15] -
                      m[9] * m[3] * m[14] - m[13] * m[2] * m[11] + m[13] * m[3] * m[10];
        inv.data[5] = m[0] * m[10] * m[15] - m[0] * m[11] * m[14] - m[8] * m[2] * m[15] +
                      m[8] * m[3] * m[14] + m[12] * m[2] * m[11] - m[12] * m[3] * m[10];
        inv.data[9] = -m[0] * m[9] * m[15] + m[0] * m[11] * m[13] + m[8] * m[1] * m[15] -
                      m[8] * m[3] * m[13] - m[12] * m[1] * m[11] + m[12] * m[3] * m[9];
        inv.data[13] = m[0] * m[9] * m[14] - m[0] * m[10] * m[13] - m[8] * m[1] * m[14] +
                       m[8] * m[2] * m[13] + m[12] * m[1] * m[10] - m[12] * m[2] * m[9];
        inv.data[2] = m[1] * m[6] * m[15] - m[1] * m[7] * m[14] - m[5] * m[2] * m[15] +
                      m[5] * m[3] * m[14] + m[13] * m[2] * m[7] - m[13] * m[3] * m[6];
        inv.data[6] = -m[0] * m[6] * m[15] + m[0] * m[7] * m[14] + m[4] * m[2] * m[15] -
                      m[4] * m[3] * m[14] - m[12] * m[2] * m[7] + m[12] * m[3] * m[6];
        inv.data[10] = m[0] * m[5] * m[15] - m[0] * m[7] * m[13] - m[4] * m[1] * m[15] +
                       m[4] * m[3] * m[13] + m[12] * m[1] * m[7] - m[12] * m[3] * m[5];
        inv.data[14] = -m[0] * m[5] * m[14] + m[0] * m[6] * m[13] + m[4] * m[1] * m[14] -
                       m[4] * m[2] * m[13] - m[12] * m[1] * m[6] + m[12] * m[2] * m[5];
        inv.data[3] = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] + m[5] * m[2] * m[11] -
                      m[5] * m[3] * m[10] - m[9] * m[2] * m[7] + m[9] * m[3] * m[6];
        inv.data[7] = m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[4] * m[2] * m[11] +
                      m[4] * m[3] * m[10] + m[8] * m[2] * m[7] - m[8] * m[3] * m[6];
        inv.data[11] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9] + m[4] * m[1] * m[11] -
                       m[4] * m[3] * m[9] - m[8] * m[1] * m[7] + m[8] * m[3] * m[5];
        inv.data[15] = m[0] * m[5] * m[10] - m[0] * m[6] * m[9] - m[4] * m[1] * m[10] +
                       m[4] * m[2] * m[9] + m[8] * m[1] * m[6] - m[8] * m[2] * m[5];

        float det = m[0] * inv.data[0] + m[1] * inv.data[4] + m[2] * inv.data[8] + m[3] * inv.data[12];
        if (std::abs(det) < 1e-6f) {
            return identity();  // Singular matrix
        }

        float inv_det = 1.0f / det;
        for (int i = 0; i < 16; ++i) {
            inv.data[i] *= inv_det;
        }
        return inv;
    }

    // ========== Construction from components ==========

    // Translation matrix
    static Mat44f translation(const Vec3& t) {
        Mat44f m = identity();
        m(3, 0) = static_cast<float>(t.x);
        m(3, 1) = static_cast<float>(t.y);
        m(3, 2) = static_cast<float>(t.z);
        return m;
    }

    static Mat44f translation(float x, float y, float z) {
        return translation(Vec3{x, y, z});
    }

    // Scale matrix
    static Mat44f scale(const Vec3& s) {
        Mat44f m;
        m(0, 0) = static_cast<float>(s.x);
        m(1, 1) = static_cast<float>(s.y);
        m(2, 2) = static_cast<float>(s.z);
        m(3, 3) = 1;
        return m;
    }

    static Mat44f scale(float s) {
        return scale(Vec3{s, s, s});
    }

    // Rotation matrix from quaternion
    static Mat44f rotation(const Quat& q) {
        Mat44f m = identity();
        float xx = static_cast<float>(q.x * q.x), yy = static_cast<float>(q.y * q.y), zz = static_cast<float>(q.z * q.z);
        float xy = static_cast<float>(q.x * q.y), xz = static_cast<float>(q.x * q.z), yz = static_cast<float>(q.y * q.z);
        float wx = static_cast<float>(q.w * q.x), wy = static_cast<float>(q.w * q.y), wz = static_cast<float>(q.w * q.z);

        m(0, 0) = 1 - 2 * (yy + zz);  m(1, 0) = 2 * (xy - wz);      m(2, 0) = 2 * (xz + wy);
        m(0, 1) = 2 * (xy + wz);      m(1, 1) = 1 - 2 * (xx + zz);  m(2, 1) = 2 * (yz - wx);
        m(0, 2) = 2 * (xz - wy);      m(1, 2) = 2 * (yz + wx);      m(2, 2) = 1 - 2 * (xx + yy);
        return m;
    }

    // Rotation around axis
    static Mat44f rotation_axis_angle(const Vec3& axis, float angle) {
        return rotation(Quat::from_axis_angle(axis, angle));
    }

    // ========== Projection matrices (Y-forward, Z-up convention) ==========

    /**
     * Perspective projection matrix.
     *
     * Camera looks along +Y axis:
     *   - View X -> Screen X (right)
     *   - View Z -> Screen Y (up)
     *   - View Y -> Depth (forward)
     *
     * @param fov_y  Vertical field of view in radians
     * @param aspect Aspect ratio (width / height)
     * @param near   Near clipping plane (must be > 0)
     * @param far    Far clipping plane (must be > near)
     */
    static Mat44f perspective(float fov_y, float aspect, float near, float far) {
        float f = 1.0f / std::tan(fov_y * 0.5f);
        Mat44f m;

        m(0, 0) = f / aspect;                           // X -> screen X
        m(2, 1) = f;                                    // Z -> screen Y (up)
        m(1, 2) = (far + near) / (far - near);          // Y -> depth
        m(3, 2) = (-2.0f * far * near) / (far - near);
        m(1, 3) = 1.0f;                                 // w = y

        return m;
    }

    // Perspective with independent horizontal and vertical FOV (may cause distortion)
    static Mat44f perspective_fov_xy(float fov_x, float fov_y, float near, float far) {
        float fx = 1.0f / std::tan(fov_x * 0.5f);
        float fy = 1.0f / std::tan(fov_y * 0.5f);
        Mat44f m;
        m(0, 0) = fx;
        m(2, 1) = fy;
        m(1, 2) = (far + near) / (far - near);
        m(3, 2) = (-2.0f * far * near) / (far - near);
        m(1, 3) = 1.0f;
        return m;
    }

    /**
     * Orthographic projection matrix.
     *
     * Camera looks along +Y axis:
     *   - View X -> Screen X (left/right)
     *   - View Z -> Screen Y (bottom/top, representing up)
     *   - View Y -> Depth (near/far)
     */
    static Mat44f orthographic(float left, float right, float bottom, float top,
                               float near, float far) {
        float lr = right - left;
        float tb = top - bottom;
        float fn = far - near;

        Mat44f m;
        m(0, 0) = 2.0f / lr;                            // X -> screen X
        m(2, 1) = 2.0f / tb;                            // Z -> screen Y (up)
        m(1, 2) = 2.0f / fn;                            // Y -> depth
        m(3, 0) = -(right + left) / lr;
        m(3, 1) = -(top + bottom) / tb;
        m(3, 2) = -(far + near) / fn;
        m(3, 3) = 1.0f;

        return m;
    }

    // ========== View matrix (Y-forward, Z-up convention) ==========

    /**
     * Look-at view matrix.
     *
     * Creates a view matrix where camera is at 'eye', looking at 'target',
     * with 'up' direction (default: +Z).
     *
     * Convention: Y-forward, Z-up
     */
    static Mat44f look_at(const Vec3& eye, const Vec3& target, const Vec3& up = Vec3::unit_z()) {
        Vec3 forward = (target - eye).normalized();  // +Y direction in camera space
        Vec3 right = forward.cross(up).normalized();  // +X direction
        Vec3 up_ortho = right.cross(forward);          // +Z direction (orthogonalized up)

        Mat44f m = identity();

        // Rotation part (transposed because we're building the inverse)
        m(0, 0) = static_cast<float>(right.x);    m(1, 0) = static_cast<float>(right.y);    m(2, 0) = static_cast<float>(right.z);
        m(0, 1) = static_cast<float>(up_ortho.x); m(1, 1) = static_cast<float>(up_ortho.y); m(2, 1) = static_cast<float>(up_ortho.z);
        m(0, 2) = static_cast<float>(forward.x);  m(1, 2) = static_cast<float>(forward.y);  m(2, 2) = static_cast<float>(forward.z);

        // Translation part
        m(3, 0) = static_cast<float>(-right.dot(eye));
        m(3, 1) = static_cast<float>(-up_ortho.dot(eye));
        m(3, 2) = static_cast<float>(-forward.dot(eye));

        return m;
    }

    // ========== Compose transform (T * R * S) ==========

    /**
     * Compose TRS matrix: Translation * Rotation * Scale
     */
    static Mat44f compose(const Vec3& t, const Quat& r, const Vec3& s) {
        Mat44f rot = rotation(r);

        // Scale the rotation columns
        for (int i = 0; i < 3; ++i) {
            rot(0, i) *= static_cast<float>(s.x);
            rot(1, i) *= static_cast<float>(s.y);
            rot(2, i) *= static_cast<float>(s.z);
        }

        // Set translation
        rot(3, 0) = static_cast<float>(t.x);
        rot(3, 1) = static_cast<float>(t.y);
        rot(3, 2) = static_cast<float>(t.z);

        return rot;
    }

    // ========== Extract components ==========

    Vec3 get_translation() const {
        return {(*this)(3, 0), (*this)(3, 1), (*this)(3, 2)};
    }

    Vec3 get_scale() const {
        float sx = std::sqrt((*this)(0, 0) * (*this)(0, 0) + (*this)(0, 1) * (*this)(0, 1) + (*this)(0, 2) * (*this)(0, 2));
        float sy = std::sqrt((*this)(1, 0) * (*this)(1, 0) + (*this)(1, 1) * (*this)(1, 1) + (*this)(1, 2) * (*this)(1, 2));
        float sz = std::sqrt((*this)(2, 0) * (*this)(2, 0) + (*this)(2, 1) * (*this)(2, 1) + (*this)(2, 2) * (*this)(2, 2));
        return {sx, sy, sz};
    }

    // Return copy with modified translation
    Mat44f with_translation(const Vec3& t) const {
        Mat44f result = *this;
        result(3, 0) = static_cast<float>(t.x);
        result(3, 1) = static_cast<float>(t.y);
        result(3, 2) = static_cast<float>(t.z);
        return result;
    }

    Mat44f with_translation(float x, float y, float z) const {
        return with_translation(Vec3{x, y, z});
    }
};


// ============================================================================
// Mat44 (double) - 4x4 Matrix in column-major order
// ============================================================================

struct Mat44 {
    double data[16];  // Column-major: [col0, col1, col2, col3]

    Mat44() { std::memset(data, 0, sizeof(data)); }

    // Access by column and row: m(col, row)
    double& operator()(int col, int row) { return data[col * 4 + row]; }
    double operator()(int col, int row) const { return data[col * 4 + row]; }

    double* ptr() { return data; }
    const double* ptr() const { return data; }

    static Mat44 identity() {
        Mat44 m;
        m(0, 0) = 1; m(1, 1) = 1; m(2, 2) = 1; m(3, 3) = 1;
        return m;
    }

    static Mat44 zero() {
        return Mat44();
    }

    // Matrix multiplication
    Mat44 operator*(const Mat44& b) const {
        Mat44 result;
        for (int col = 0; col < 4; ++col) {
            for (int row = 0; row < 4; ++row) {
                double sum = 0;
                for (int k = 0; k < 4; ++k) {
                    sum += (*this)(k, row) * b(col, k);
                }
                result(col, row) = sum;
            }
        }
        return result;
    }

    // Transform point (w=1)
    Vec3 transform_point(const Vec3& p) const {
        double x = (*this)(0, 0) * p.x + (*this)(1, 0) * p.y + (*this)(2, 0) * p.z + (*this)(3, 0);
        double y = (*this)(0, 1) * p.x + (*this)(1, 1) * p.y + (*this)(2, 1) * p.z + (*this)(3, 1);
        double z = (*this)(0, 2) * p.x + (*this)(1, 2) * p.y + (*this)(2, 2) * p.z + (*this)(3, 2);
        double w = (*this)(0, 3) * p.x + (*this)(1, 3) * p.y + (*this)(2, 3) * p.z + (*this)(3, 3);
        if (std::abs(w) > 1e-10) {
            return {x / w, y / w, z / w};
        }
        return {x, y, z};
    }

    // Transform direction (w=0)
    Vec3 transform_direction(const Vec3& d) const {
        return {
            (*this)(0, 0) * d.x + (*this)(1, 0) * d.y + (*this)(2, 0) * d.z,
            (*this)(0, 1) * d.x + (*this)(1, 1) * d.y + (*this)(2, 1) * d.z,
            (*this)(0, 2) * d.x + (*this)(1, 2) * d.y + (*this)(2, 2) * d.z
        };
    }

    // Transpose
    Mat44 transposed() const {
        Mat44 result;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                result(i, j) = (*this)(j, i);
            }
        }
        return result;
    }

    // Inverse
    Mat44 inverse() const {
        Mat44 inv;
        const double* m = data;

        inv.data[0] = m[5] * m[10] * m[15] - m[5] * m[11] * m[14] - m[9] * m[6] * m[15] +
                      m[9] * m[7] * m[14] + m[13] * m[6] * m[11] - m[13] * m[7] * m[10];
        inv.data[4] = -m[4] * m[10] * m[15] + m[4] * m[11] * m[14] + m[8] * m[6] * m[15] -
                      m[8] * m[7] * m[14] - m[12] * m[6] * m[11] + m[12] * m[7] * m[10];
        inv.data[8] = m[4] * m[9] * m[15] - m[4] * m[11] * m[13] - m[8] * m[5] * m[15] +
                      m[8] * m[7] * m[13] + m[12] * m[5] * m[11] - m[12] * m[7] * m[9];
        inv.data[12] = -m[4] * m[9] * m[14] + m[4] * m[10] * m[13] + m[8] * m[5] * m[14] -
                       m[8] * m[6] * m[13] - m[12] * m[5] * m[10] + m[12] * m[6] * m[9];
        inv.data[1] = -m[1] * m[10] * m[15] + m[1] * m[11] * m[14] + m[9] * m[2] * m[15] -
                      m[9] * m[3] * m[14] - m[13] * m[2] * m[11] + m[13] * m[3] * m[10];
        inv.data[5] = m[0] * m[10] * m[15] - m[0] * m[11] * m[14] - m[8] * m[2] * m[15] +
                      m[8] * m[3] * m[14] + m[12] * m[2] * m[11] - m[12] * m[3] * m[10];
        inv.data[9] = -m[0] * m[9] * m[15] + m[0] * m[11] * m[13] + m[8] * m[1] * m[15] -
                      m[8] * m[3] * m[13] - m[12] * m[1] * m[11] + m[12] * m[3] * m[9];
        inv.data[13] = m[0] * m[9] * m[14] - m[0] * m[10] * m[13] - m[8] * m[1] * m[14] +
                       m[8] * m[2] * m[13] + m[12] * m[1] * m[10] - m[12] * m[2] * m[9];
        inv.data[2] = m[1] * m[6] * m[15] - m[1] * m[7] * m[14] - m[5] * m[2] * m[15] +
                      m[5] * m[3] * m[14] + m[13] * m[2] * m[7] - m[13] * m[3] * m[6];
        inv.data[6] = -m[0] * m[6] * m[15] + m[0] * m[7] * m[14] + m[4] * m[2] * m[15] -
                      m[4] * m[3] * m[14] - m[12] * m[2] * m[7] + m[12] * m[3] * m[6];
        inv.data[10] = m[0] * m[5] * m[15] - m[0] * m[7] * m[13] - m[4] * m[1] * m[15] +
                       m[4] * m[3] * m[13] + m[12] * m[1] * m[7] - m[12] * m[3] * m[5];
        inv.data[14] = -m[0] * m[5] * m[14] + m[0] * m[6] * m[13] + m[4] * m[1] * m[14] -
                       m[4] * m[2] * m[13] - m[12] * m[1] * m[6] + m[12] * m[2] * m[5];
        inv.data[3] = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] + m[5] * m[2] * m[11] -
                      m[5] * m[3] * m[10] - m[9] * m[2] * m[7] + m[9] * m[3] * m[6];
        inv.data[7] = m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[4] * m[2] * m[11] +
                      m[4] * m[3] * m[10] + m[8] * m[2] * m[7] - m[8] * m[3] * m[6];
        inv.data[11] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9] + m[4] * m[1] * m[11] -
                       m[4] * m[3] * m[9] - m[8] * m[1] * m[7] + m[8] * m[3] * m[5];
        inv.data[15] = m[0] * m[5] * m[10] - m[0] * m[6] * m[9] - m[4] * m[1] * m[10] +
                       m[4] * m[2] * m[9] + m[8] * m[1] * m[6] - m[8] * m[2] * m[5];

        double det = m[0] * inv.data[0] + m[1] * inv.data[4] + m[2] * inv.data[8] + m[3] * inv.data[12];
        if (std::abs(det) < 1e-10) {
            return identity();
        }

        double inv_det = 1.0 / det;
        for (int i = 0; i < 16; ++i) {
            inv.data[i] *= inv_det;
        }
        return inv;
    }

    // Conversion to float
    Mat44f to_float() const {
        Mat44f m;
        for (int i = 0; i < 16; ++i) {
            m.data[i] = static_cast<float>(data[i]);
        }
        return m;
    }

    // ========== Construction from components ==========

    static Mat44 translation(const Vec3& t) {
        Mat44 m = identity();
        m(3, 0) = t.x;
        m(3, 1) = t.y;
        m(3, 2) = t.z;
        return m;
    }

    static Mat44 translation(double x, double y, double z) {
        return translation(Vec3{x, y, z});
    }

    static Mat44 scale(const Vec3& s) {
        Mat44 m;
        m(0, 0) = s.x;
        m(1, 1) = s.y;
        m(2, 2) = s.z;
        m(3, 3) = 1;
        return m;
    }

    static Mat44 scale(double s) {
        return scale(Vec3{s, s, s});
    }

    static Mat44 rotation(const Quat& q) {
        Mat44 m = identity();
        double xx = q.x * q.x, yy = q.y * q.y, zz = q.z * q.z;
        double xy = q.x * q.y, xz = q.x * q.z, yz = q.y * q.z;
        double wx = q.w * q.x, wy = q.w * q.y, wz = q.w * q.z;

        m(0, 0) = 1 - 2 * (yy + zz);  m(1, 0) = 2 * (xy - wz);      m(2, 0) = 2 * (xz + wy);
        m(0, 1) = 2 * (xy + wz);      m(1, 1) = 1 - 2 * (xx + zz);  m(2, 1) = 2 * (yz - wx);
        m(0, 2) = 2 * (xz - wy);      m(1, 2) = 2 * (yz + wx);      m(2, 2) = 1 - 2 * (xx + yy);
        return m;
    }

    static Mat44 rotation_axis_angle(const Vec3& axis, double angle) {
        return rotation(Quat::from_axis_angle(axis, angle));
    }

    static Mat44 perspective(double fov_y, double aspect, double near, double far) {
        double f = 1.0 / std::tan(fov_y * 0.5);
        Mat44 m;
        m(0, 0) = f / aspect;
        m(2, 1) = f;
        m(1, 2) = (far + near) / (far - near);
        m(3, 2) = (-2.0 * far * near) / (far - near);
        m(1, 3) = 1.0;
        return m;
    }

    // Perspective with independent horizontal and vertical FOV (may cause distortion)
    static Mat44 perspective_fov_xy(double fov_x, double fov_y, double near, double far) {
        double fx = 1.0 / std::tan(fov_x * 0.5);
        double fy = 1.0 / std::tan(fov_y * 0.5);
        Mat44 m;
        m(0, 0) = fx;
        m(2, 1) = fy;
        m(1, 2) = (far + near) / (far - near);
        m(3, 2) = (-2.0 * far * near) / (far - near);
        m(1, 3) = 1.0;
        return m;
    }

    static Mat44 orthographic(double left, double right, double bottom, double top,
                              double near, double far) {
        double lr = right - left;
        double tb = top - bottom;
        double fn = far - near;

        Mat44 m;
        m(0, 0) = 2.0 / lr;
        m(2, 1) = 2.0 / tb;
        m(1, 2) = 2.0 / fn;
        m(3, 0) = -(right + left) / lr;
        m(3, 1) = -(top + bottom) / tb;
        m(3, 2) = -(far + near) / fn;
        m(3, 3) = 1.0;
        return m;
    }

    static Mat44 look_at(const Vec3& eye, const Vec3& target, const Vec3& up = Vec3::unit_z()) {
        Vec3 forward = (target - eye).normalized();
        Vec3 right = forward.cross(up).normalized();
        Vec3 up_ortho = right.cross(forward);

        Mat44 m = identity();
        m(0, 0) = right.x;    m(1, 0) = right.y;    m(2, 0) = right.z;
        m(0, 1) = up_ortho.x; m(1, 1) = up_ortho.y; m(2, 1) = up_ortho.z;
        m(0, 2) = forward.x;  m(1, 2) = forward.y;  m(2, 2) = forward.z;
        m(3, 0) = -right.dot(eye);
        m(3, 1) = -up_ortho.dot(eye);
        m(3, 2) = -forward.dot(eye);
        return m;
    }

    static Mat44 compose(const Vec3& t, const Quat& r, const Vec3& s) {
        Mat44 rot = rotation(r);
        for (int i = 0; i < 3; ++i) {
            rot(0, i) *= s.x;
            rot(1, i) *= s.y;
            rot(2, i) *= s.z;
        }
        rot(3, 0) = t.x;
        rot(3, 1) = t.y;
        rot(3, 2) = t.z;
        return rot;
    }

    Vec3 get_translation() const {
        return {(*this)(3, 0), (*this)(3, 1), (*this)(3, 2)};
    }

    Vec3 get_scale() const {
        double sx = std::sqrt((*this)(0, 0) * (*this)(0, 0) + (*this)(0, 1) * (*this)(0, 1) + (*this)(0, 2) * (*this)(0, 2));
        double sy = std::sqrt((*this)(1, 0) * (*this)(1, 0) + (*this)(1, 1) * (*this)(1, 1) + (*this)(1, 2) * (*this)(1, 2));
        double sz = std::sqrt((*this)(2, 0) * (*this)(2, 0) + (*this)(2, 1) * (*this)(2, 1) + (*this)(2, 2) * (*this)(2, 2));
        return {sx, sy, sz};
    }

    // Return copy with modified translation
    Mat44 with_translation(const Vec3& t) const {
        Mat44 result = *this;
        result(3, 0) = t.x;
        result(3, 1) = t.y;
        result(3, 2) = t.z;
        return result;
    }

    Mat44 with_translation(double x, double y, double z) const {
        return with_translation(Vec3{x, y, z});
    }
};


} // namespace termin
