#pragma once

#include "vec3.hpp"
#include "quat.hpp"
#include "mat44.hpp"
#include <cmath>
#include <algorithm>

namespace termin {


struct Pose3 {
    Quat ang;  // Rotation (quaternion)
    Vec3 lin;  // Translation

    Pose3() : ang(Quat::identity()), lin(Vec3::zero()) {}
    Pose3(const Quat& ang, const Vec3& lin) : ang(ang), lin(lin) {}

    static Pose3 identity() { return {}; }

    // SE(3) composition: this * other
    Pose3 operator*(const Pose3& other) const {
        return {
            ang * other.ang,
            lin + ang.rotate(other.lin)
        };
    }

    // Inverse pose
    Pose3 inverse() const {
        Quat inv_ang = ang.inverse();
        return {inv_ang, inv_ang.rotate(-lin)};
    }

    // Transform point: R * p + t
    Vec3 transform_point(const Vec3& p) const {
        return ang.rotate(p) + lin;
    }

    // Transform vector (rotation only)
    Vec3 transform_vector(const Vec3& v) const {
        return ang.rotate(v);
    }

    // Rotate point (alias for transform_vector)
    Vec3 rotate_point(const Vec3& p) const {
        return ang.rotate(p);
    }

    // Inverse transform point: R^T * (p - t)
    Vec3 inverse_transform_point(const Vec3& p) const {
        return ang.inverse_rotate(p - lin);
    }

    // Inverse transform vector
    Vec3 inverse_transform_vector(const Vec3& v) const {
        return ang.inverse_rotate(v);
    }

    // Normalize quaternion
    Pose3 normalized() const {
        return {ang.normalized(), lin};
    }

    // With new translation
    Pose3 with_translation(const Vec3& new_lin) const {
        return {ang, new_lin};
    }

    // With new rotation
    Pose3 with_rotation(const Quat& new_ang) const {
        return {new_ang, lin};
    }

    // Get 3x3 rotation matrix
    void rotation_matrix(double* m) const {
        ang.to_matrix(m);
    }

    // Get 4x4 transformation matrix (column-major)
    void as_matrix(double* m) const {
        double rot[9];
        ang.to_matrix(rot);
        // Column 0
        m[0] = rot[0]; m[1] = rot[3]; m[2] = rot[6]; m[3] = 0;
        // Column 1
        m[4] = rot[1]; m[5] = rot[4]; m[6] = rot[7]; m[7] = 0;
        // Column 2
        m[8] = rot[2]; m[9] = rot[5]; m[10] = rot[8]; m[11] = 0;
        // Column 3
        m[12] = lin.x; m[13] = lin.y; m[14] = lin.z; m[15] = 1;
    }

    // Get 4x4 transformation matrix as Mat44
    Mat44 as_mat44() const {
        Mat44 m;
        as_matrix(m.data);
        return m;
    }

    // Distance between translations
    double distance(const Pose3& other) const {
        return (lin - other.lin).norm();
    }

    // Factory methods
    static Pose3 translation(double x, double y, double z) {
        return {Quat::identity(), {x, y, z}};
    }

    static Pose3 translation(const Vec3& t) {
        return {Quat::identity(), t};
    }

    static Pose3 rotation(const Vec3& axis, double angle) {
        return {Quat::from_axis_angle(axis, angle), Vec3::zero()};
    }

    static Pose3 rotate_x(double angle) {
        return rotation(Vec3::unit_x(), angle);
    }

    static Pose3 rotate_y(double angle) {
        return rotation(Vec3::unit_y(), angle);
    }

    static Pose3 rotate_z(double angle) {
        return rotation(Vec3::unit_z(), angle);
    }

    // Looking at (Y-forward convention)
    static Pose3 looking_at(const Vec3& eye, const Vec3& target, const Vec3& up = Vec3::unit_z()) {
        Vec3 forward = (target - eye).normalized();
        Vec3 right = forward.cross(up).normalized();
        Vec3 up_corrected = right.cross(forward);

        // Rotation matrix: local X=right, Y=forward, Z=up
        // Convert to quaternion using Shepperd's method
        double m00 = right.x, m01 = forward.x, m02 = up_corrected.x;
        double m10 = right.y, m11 = forward.y, m12 = up_corrected.y;
        double m20 = right.z, m21 = forward.z, m22 = up_corrected.z;

        double trace = m00 + m11 + m22;
        Quat q;

        if (trace > 0) {
            double s = 0.5 / std::sqrt(trace + 1.0);
            q = Quat(
                (m21 - m12) * s,
                (m02 - m20) * s,
                (m10 - m01) * s,
                0.25 / s
            );
        } else if (m00 > m11 && m00 > m22) {
            double s = 2.0 * std::sqrt(1.0 + m00 - m11 - m22);
            q = Quat(
                0.25 * s,
                (m01 + m10) / s,
                (m02 + m20) / s,
                (m21 - m12) / s
            );
        } else if (m11 > m22) {
            double s = 2.0 * std::sqrt(1.0 + m11 - m00 - m22);
            q = Quat(
                (m01 + m10) / s,
                0.25 * s,
                (m12 + m21) / s,
                (m02 - m20) / s
            );
        } else {
            double s = 2.0 * std::sqrt(1.0 + m22 - m00 - m11);
            q = Quat(
                (m02 + m20) / s,
                (m12 + m21) / s,
                0.25 * s,
                (m10 - m01) / s
            );
        }

        return {q.normalized(), eye};
    }

    // From Euler angles (XYZ order)
    static Pose3 from_euler(double roll, double pitch, double yaw) {
        double cr = std::cos(roll * 0.5);
        double sr = std::sin(roll * 0.5);
        double cp = std::cos(pitch * 0.5);
        double sp = std::sin(pitch * 0.5);
        double cy = std::cos(yaw * 0.5);
        double sy = std::sin(yaw * 0.5);

        Quat q(
            sr * cp * cy - cr * sp * sy,
            cr * sp * cy + sr * cp * sy,
            cr * cp * sy - sr * sp * cy,
            cr * cp * cy + sr * sp * sy
        );
        return {q, Vec3::zero()};
    }

    // To Euler angles (XYZ order)
    Vec3 to_euler() const {
        double x = ang.x, y = ang.y, z = ang.z, w = ang.w;

        // Roll (x-axis rotation)
        double sinr_cosp = 2 * (w * x + y * z);
        double cosr_cosp = 1 - 2 * (x * x + y * y);
        double roll = std::atan2(sinr_cosp, cosr_cosp);

        // Pitch (y-axis rotation)
        double sinp = 2 * (w * y - z * x);
        if (sinp > 1) sinp = 1;
        if (sinp < -1) sinp = -1;
        double pitch = std::asin(sinp);

        // Yaw (z-axis rotation)
        double siny_cosp = 2 * (w * z + x * y);
        double cosy_cosp = 1 - 2 * (y * y + z * z);
        double yaw = std::atan2(siny_cosp, cosy_cosp);

        return {roll, pitch, yaw};
    }

    // To axis-angle
    void to_axis_angle(Vec3& axis, double& angle) const {
        angle = 2 * std::acos(std::max(-1.0, std::min(1.0, ang.w)));
        double s = std::sqrt(1 - ang.w * ang.w);
        if (s < 0.001) {
            axis = Vec3::unit_x();
        } else {
            axis = Vec3(ang.x / s, ang.y / s, ang.z / s);
        }
    }

    // Copy
    Pose3 copy() const { return *this; }
};

// Linear interpolation
inline Pose3 lerp(const Pose3& p1, const Pose3& p2, double t) {
    return {
        slerp(p1.ang, p2.ang, t),
        p1.lin + (p2.lin - p1.lin) * t
    };
}


} // namespace termin
