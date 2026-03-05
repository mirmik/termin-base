#pragma once

#include <cmath>
#include "vec3.hpp"
#include "quat.hpp"
#include "pose3.hpp"

namespace termin {


struct GeneralPose3 {
    Quat ang;   // Rotation quaternion (x, y, z, w)
    Vec3 lin;   // Translation
    Vec3 scale; // Per-axis scale

    GeneralPose3()
        : ang(Quat::identity())
        , lin(Vec3::zero())
        , scale(1.0, 1.0, 1.0) {}

    GeneralPose3(const Quat& ang, const Vec3& lin, const Vec3& scale = Vec3{1.0, 1.0, 1.0})
        : ang(ang)
        , lin(lin)
        , scale(scale) {}

    static GeneralPose3 identity() { return {}; }

    // --- Core ops ------------------------------------------------------------

    // Composition with scale inheritance:
    // new_lin   = parent.lin + R_parent * (parent.scale ⊙ child.lin)
    // new_ang   = parent.ang * child.ang
    // new_scale = parent.scale ⊙ child.scale
    GeneralPose3 operator*(const GeneralPose3& other) const {
        Vec3 scaled_child{scale.x * other.lin.x, scale.y * other.lin.y, scale.z * other.lin.z};
        Vec3 rotated_child = ang.rotate(scaled_child);
        return {
            ang * other.ang,
            lin + rotated_child,
            {scale.x * other.scale.x, scale.y * other.scale.y, scale.z * other.scale.z}
        };
    }

    // Composition with Pose3 (treated as unit scale)
    GeneralPose3 operator*(const Pose3& other) const {
        Vec3 scaled_child{scale.x * other.lin.x, scale.y * other.lin.y, scale.z * other.lin.z};
        Vec3 rotated_child = ang.rotate(scaled_child);
        return {
            ang * other.ang,
            lin + rotated_child,
            scale  // Pose3 has unit scale, so preserve this->scale
        };
    }

    // Inverse: S^-1 * R^-1 * T^-1
    GeneralPose3 inverse() const {
        Quat inv_ang = ang.inverse();
        Vec3 inv_scale{
            scale.x != 0.0 ? 1.0 / scale.x : 0.0,
            scale.y != 0.0 ? 1.0 / scale.y : 0.0,
            scale.z != 0.0 ? 1.0 / scale.z : 0.0
        };
        Vec3 inv_lin = inv_ang.rotate(-lin);
        inv_lin.x *= inv_scale.x;
        inv_lin.y *= inv_scale.y;
        inv_lin.z *= inv_scale.z;
        return {inv_ang, inv_lin, inv_scale};
    }

    // Apply TRS to a point: R * (S ⊙ p) + t
    Vec3 transform_point(const Vec3& p) const {
        Vec3 scaled{scale.x * p.x, scale.y * p.y, scale.z * p.z};
        return ang.rotate(scaled) + lin;
    }

    // Apply rotation+scale only (no translation)
    Vec3 transform_vector(const Vec3& v) const {
        Vec3 scaled{scale.x * v.x, scale.y * v.y, scale.z * v.z};
        return ang.rotate(scaled);
    }

    // Alias for transform_vector
    Vec3 rotate_point(const Vec3& p) const {
        return transform_vector(p);
    }

    // Inverse transform: S^-1 * R^T * (p - t)
    Vec3 inverse_transform_point(const Vec3& p) const {
        Vec3 inv_scale{
            scale.x != 0.0 ? 1.0 / scale.x : 0.0,
            scale.y != 0.0 ? 1.0 / scale.y : 0.0,
            scale.z != 0.0 ? 1.0 / scale.z : 0.0
        };
        Vec3 diff = p - lin;
        Vec3 rot = ang.inverse_rotate(diff);
        rot.x *= inv_scale.x;
        rot.y *= inv_scale.y;
        rot.z *= inv_scale.z;
        return rot;
    }

    // Inverse transform for vectors
    Vec3 inverse_transform_vector(const Vec3& v) const {
        Vec3 inv_scale{
            scale.x != 0.0 ? 1.0 / scale.x : 0.0,
            scale.y != 0.0 ? 1.0 / scale.y : 0.0,
            scale.z != 0.0 ? 1.0 / scale.z : 0.0
        };
        Vec3 rot = ang.inverse_rotate(v);
        rot.x *= inv_scale.x;
        rot.y *= inv_scale.y;
        rot.z *= inv_scale.z;
        return rot;
    }

    GeneralPose3 normalized() const {
        return {ang.normalized(), lin, scale};
    }

    GeneralPose3 with_rotation(const Quat& new_ang) const {
        return {new_ang, lin, scale};
    }

    GeneralPose3 with_translation(const Vec3& new_lin) const {
        return {ang, new_lin, scale};
    }

    GeneralPose3 with_scale(const Vec3& new_scale) const {
        return {ang, lin, new_scale};
    }

    Pose3 to_pose3() const {
        return {ang, lin};
    }

    // --- Matrices (column-major, OpenGL convention) ---------------------------

    // 3x3 rotation matrix
    void rotation_matrix(double* m) const {
        ang.to_matrix(m);
    }

    // 4x4 TRS matrix (column-major)
    void matrix4(double* m) const {
        double r[9];
        rotation_matrix(r);
        // Column 0
        m[0] = r[0] * scale.x; m[1] = r[3] * scale.x; m[2] = r[6] * scale.x; m[3] = 0.0;
        // Column 1
        m[4] = r[1] * scale.y; m[5] = r[4] * scale.y; m[6] = r[7] * scale.y; m[7] = 0.0;
        // Column 2
        m[8] = r[2] * scale.z; m[9] = r[5] * scale.z; m[10] = r[8] * scale.z; m[11] = 0.0;
        // Column 3
        m[12] = lin.x; m[13] = lin.y; m[14] = lin.z; m[15] = 1.0;
    }

    // 3x4 TRS matrix (column-major, columns 0-2 are rotation*scale, column 3 is translation)
    void matrix34(double* m) const {
        double r[9];
        rotation_matrix(r);
        // Column 0
        m[0] = r[0] * scale.x; m[1] = r[3] * scale.x; m[2] = r[6] * scale.x;
        // Column 1
        m[3] = r[1] * scale.y; m[4] = r[4] * scale.y; m[5] = r[7] * scale.y;
        // Column 2
        m[6] = r[2] * scale.z; m[7] = r[5] * scale.z; m[8] = r[8] * scale.z;
        // Column 3
        m[9] = lin.x; m[10] = lin.y; m[11] = lin.z;
    }

    // Inverse 4x4 matrix: S^-1 @ R^T @ T^-1 (column-major)
    void inverse_matrix4(double* m) const {
        double r[9];
        rotation_matrix(r);

        double inv_sx = scale.x != 0.0 ? 1.0 / scale.x : 0.0;
        double inv_sy = scale.y != 0.0 ? 1.0 / scale.y : 0.0;
        double inv_sz = scale.z != 0.0 ? 1.0 / scale.z : 0.0;

        // R^T scaled by S^-1 (rows scaled)
        double m00 = inv_sx * r[0];
        double m01 = inv_sx * r[3];
        double m02 = inv_sx * r[6];

        double m10 = inv_sy * r[1];
        double m11 = inv_sy * r[4];
        double m12 = inv_sy * r[7];

        double m20 = inv_sz * r[2];
        double m21 = inv_sz * r[5];
        double m22 = inv_sz * r[8];

        // -S^-1 R^T t
        double tx = -(m00 * lin.x + m01 * lin.y + m02 * lin.z);
        double ty = -(m10 * lin.x + m11 * lin.y + m12 * lin.z);
        double tz = -(m20 * lin.x + m21 * lin.y + m22 * lin.z);

        // Column 0
        m[0] = m00; m[1] = m10; m[2] = m20; m[3] = 0.0;
        // Column 1
        m[4] = m01; m[5] = m11; m[6] = m21; m[7] = 0.0;
        // Column 2
        m[8] = m02; m[9] = m12; m[10] = m22; m[11] = 0.0;
        // Column 3
        m[12] = tx; m[13] = ty; m[14] = tz; m[15] = 1.0;
    }

    // --- Helpers ------------------------------------------------------------

    double distance(const GeneralPose3& other) const {
        return (lin - other.lin).norm();
    }

    static GeneralPose3 translation(double x, double y, double z) {
        return {Quat::identity(), {x, y, z}, {1.0, 1.0, 1.0}};
    }

    static GeneralPose3 translation(const Vec3& t) {
        return {Quat::identity(), t, {1.0, 1.0, 1.0}};
    }

    static GeneralPose3 rotation(const Vec3& axis, double angle) {
        return {Quat::from_axis_angle(axis, angle), Vec3::zero(), {1.0, 1.0, 1.0}};
    }

    static GeneralPose3 scaling(double sx, double sy, double sz) {
        return {Quat::identity(), Vec3::zero(), {sx, sy, sz}};
    }

    static GeneralPose3 scaling(double s) {
        return {Quat::identity(), Vec3::zero(), {s, s, s}};
    }

    static GeneralPose3 rotate_x(double angle) {
        return rotation(Vec3::unit_x(), angle);
    }

    static GeneralPose3 rotate_y(double angle) {
        return rotation(Vec3::unit_y(), angle);
    }

    static GeneralPose3 rotate_z(double angle) {
        return rotation(Vec3::unit_z(), angle);
    }

    static GeneralPose3 move(double dx, double dy, double dz) {
        return translation(dx, dy, dz);
    }

    static GeneralPose3 move_x(double d) { return move(d, 0.0, 0.0); }
    static GeneralPose3 move_y(double d) { return move(0.0, d, 0.0); }
    static GeneralPose3 move_z(double d) { return move(0.0, 0.0, d); }

    static GeneralPose3 right(double d)   { return move_x(d); }
    static GeneralPose3 forward(double d) { return move_y(d); }
    static GeneralPose3 up(double d)      { return move_z(d); }

    // Y-forward look-at (X=right, Y=forward, Z=up)
    static GeneralPose3 looking_at(const Vec3& eye, const Vec3& target, const Vec3& up_vec = Vec3{0.0, 0.0, 1.0}) {
        Vec3 forward_vec = (target - eye).normalized();
        Vec3 right_vec = forward_vec.cross(up_vec).normalized();
        Vec3 up_corrected = right_vec.cross(forward_vec);

        // Rotation matrix with columns [right, forward, up]
        double r00 = right_vec.x,   r01 = forward_vec.x,   r02 = up_corrected.x;
        double r10 = right_vec.y,   r11 = forward_vec.y,   r12 = up_corrected.y;
        double r20 = right_vec.z,   r21 = forward_vec.z,   r22 = up_corrected.z;

        double trace = r00 + r11 + r22;
        Quat q;
        if (trace > 0.0) {
            double s = 0.5 / std::sqrt(trace + 1.0);
            q.w = 0.25 / s;
            q.x = (r21 - r12) * s;
            q.y = (r02 - r20) * s;
            q.z = (r10 - r01) * s;
        } else if (r00 > r11 && r00 > r22) {
            double s = 2.0 * std::sqrt(1.0 + r00 - r11 - r22);
            q.w = (r21 - r12) / s;
            q.x = 0.25 * s;
            q.y = (r01 + r10) / s;
            q.z = (r02 + r20) / s;
        } else if (r11 > r22) {
            double s = 2.0 * std::sqrt(1.0 + r11 - r00 - r22);
            q.w = (r02 - r20) / s;
            q.x = (r01 + r10) / s;
            q.y = 0.25 * s;
            q.z = (r12 + r21) / s;
        } else {
            double s = 2.0 * std::sqrt(1.0 + r22 - r00 - r11);
            q.w = (r10 - r01) / s;
            q.x = (r02 + r20) / s;
            q.y = (r12 + r21) / s;
            q.z = 0.25 * s;
        }

        return {q, eye, {1.0, 1.0, 1.0}};
    }
};

// Linear interpolation of translation/scale + slerp rotation
inline GeneralPose3 lerp(const GeneralPose3& a, const GeneralPose3& b, double t) {
    return {
        slerp(a.ang, b.ang, t),
        a.lin + (b.lin - a.lin) * t,
        {a.scale.x + (b.scale.x - a.scale.x) * t,
         a.scale.y + (b.scale.y - a.scale.y) * t,
         a.scale.z + (b.scale.z - a.scale.z) * t}
    };
}

// Pose3 * GeneralPose3 (Pose3 treated as unit scale)
inline GeneralPose3 operator*(const Pose3& a, const GeneralPose3& b) {
    Vec3 rotated_child = a.ang.rotate(b.lin);
    return {
        a.ang * b.ang,
        a.lin + rotated_child,
        b.scale  // Pose3 has unit scale
    };
}

} // namespace termin

