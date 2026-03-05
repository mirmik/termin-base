#pragma once

#include "vec3.hpp"
#include <cmath>

namespace termin {


struct Quat {
    double x, y, z, w;  // (x, y, z, w) format

    Quat() : x(0), y(0), z(0), w(1) {}
    Quat(double x, double y, double z, double w) : x(x), y(y), z(z), w(w) {}

    static Quat identity() { return {0, 0, 0, 1}; }

    // Quaternion multiplication
    Quat operator*(const Quat& q) const {
        return {
            w * q.x + x * q.w + y * q.z - z * q.y,
            w * q.y - x * q.z + y * q.w + z * q.x,
            w * q.z + x * q.y - y * q.x + z * q.w,
            w * q.w - x * q.x - y * q.y - z * q.z
        };
    }

    Quat conjugate() const { return {-x, -y, -z, w}; }
    Quat inverse() const { return conjugate(); }  // Assumes unit quaternion

    double norm() const { return std::sqrt(x * x + y * y + z * z + w * w); }

    Quat normalized() const {
        double n = norm();
        return n > 1e-10 ? Quat{x / n, y / n, z / n, w / n} : identity();
    }

    // Rotate vector by quaternion (optimized formula)
    Vec3 rotate(const Vec3& v) const {
        // t = 2 * cross(q.xyz, v)
        double tx = 2.0 * (y * v.z - z * v.y);
        double ty = 2.0 * (z * v.x - x * v.z);
        double tz = 2.0 * (x * v.y - y * v.x);

        // result = v + w * t + cross(q.xyz, t)
        return {
            v.x + w * tx + y * tz - z * ty,
            v.y + w * ty + z * tx - x * tz,
            v.z + w * tz + x * ty - y * tx
        };
    }

    // Inverse rotate
    Vec3 inverse_rotate(const Vec3& v) const {
        return conjugate().rotate(v);
    }

    // Create from axis-angle
    static Quat from_axis_angle(const Vec3& axis, double angle) {
        double half = angle * 0.5;
        double s = std::sin(half);
        Vec3 n = axis.normalized();
        return {n.x * s, n.y * s, n.z * s, std::cos(half)};
    }

    // Create quaternion that looks in direction
    // Convention: Forward = +Y, Up = +Z
    // forward: the direction to look at
    // up: the up vector (default Z-up)
    static Quat look_rotation(const Vec3& forward, const Vec3& up = Vec3::unit_z()) {
        Vec3 f = forward.normalized();
        Vec3 r = f.cross(up).normalized();  // right = forward x up
        Vec3 u = r.cross(f);                // corrected up = right x forward

        // Build rotation matrix where:
        // X-axis = right, Y-axis = forward, Z-axis = up
        // Row-major: m[row*3 + col]
        double m[9] = {
            r.x, f.x, u.x,
            r.y, f.y, u.y,
            r.z, f.z, u.z
        };
        return from_rotation_matrix(m);
    }

    // Static slerp
    static Quat slerp(const Quat& a, const Quat& b, double t);

    // Create from 3x3 rotation matrix (row-major: m[row*3+col])
    static Quat from_rotation_matrix(const double* m) {
        double trace = m[0] + m[4] + m[8];
        double x, y, z, w;

        if (trace > 0) {
            double s = 0.5 / std::sqrt(trace + 1.0);
            w = 0.25 / s;
            x = (m[7] - m[5]) * s;
            y = (m[2] - m[6]) * s;
            z = (m[3] - m[1]) * s;
        } else if (m[0] > m[4] && m[0] > m[8]) {
            double s = 2.0 * std::sqrt(1.0 + m[0] - m[4] - m[8]);
            w = (m[7] - m[5]) / s;
            x = 0.25 * s;
            y = (m[1] + m[3]) / s;
            z = (m[2] + m[6]) / s;
        } else if (m[4] > m[8]) {
            double s = 2.0 * std::sqrt(1.0 + m[4] - m[0] - m[8]);
            w = (m[2] - m[6]) / s;
            x = (m[1] + m[3]) / s;
            y = 0.25 * s;
            z = (m[5] + m[7]) / s;
        } else {
            double s = 2.0 * std::sqrt(1.0 + m[8] - m[0] - m[4]);
            w = (m[3] - m[1]) / s;
            x = (m[2] + m[6]) / s;
            y = (m[5] + m[7]) / s;
            z = 0.25 * s;
        }

        return Quat{x, y, z, w}.normalized();
    }

    // To 3x3 rotation matrix (row-major)
    void to_matrix(double* m) const {
        double xx = x * x, yy = y * y, zz = z * z;
        double xy = x * y, xz = x * z, yz = y * z;
        double wx = w * x, wy = w * y, wz = w * z;

        m[0] = 1 - 2 * (yy + zz);  m[1] = 2 * (xy - wz);      m[2] = 2 * (xz + wy);
        m[3] = 2 * (xy + wz);      m[4] = 1 - 2 * (xx + zz);  m[5] = 2 * (yz - wx);
        m[6] = 2 * (xz - wy);      m[7] = 2 * (yz + wx);      m[8] = 1 - 2 * (xx + yy);
    }
};

// Spherical linear interpolation
inline Quat slerp(const Quat& q1, Quat q2, double t) {
    double dot = q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;

    if (dot < 0) {
        q2 = {-q2.x, -q2.y, -q2.z, -q2.w};
        dot = -dot;
    }

    if (dot > 0.9995) {
        // Linear interpolation for close quaternions
        Quat result = {
            q1.x + t * (q2.x - q1.x),
            q1.y + t * (q2.y - q1.y),
            q1.z + t * (q2.z - q1.z),
            q1.w + t * (q2.w - q1.w)
        };
        return result.normalized();
    }

    double theta_0 = std::acos(dot);
    double theta = theta_0 * t;
    double sin_theta = std::sin(theta);
    double sin_theta_0 = std::sin(theta_0);

    double s1 = std::cos(theta) - dot * sin_theta / sin_theta_0;
    double s2 = sin_theta / sin_theta_0;

    return {
        s1 * q1.x + s2 * q2.x,
        s1 * q1.y + s2 * q2.y,
        s1 * q1.z + s2 * q2.z,
        s1 * q1.w + s2 * q2.w
    };
}

// Static member implementation
inline Quat Quat::slerp(const Quat& a, const Quat& b, double t) {
    return termin::slerp(a, b, t);
}


} // namespace termin
