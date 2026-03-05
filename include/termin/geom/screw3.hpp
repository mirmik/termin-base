#pragma once

#include "vec3.hpp"
#include "pose3.hpp"
#include <cmath>

namespace termin {


struct Screw3 {
    Vec3 ang;  // Angular part (omega)
    Vec3 lin;  // Linear part (v)

    Screw3() : ang(Vec3::zero()), lin(Vec3::zero()) {}
    Screw3(const Vec3& ang, const Vec3& lin) : ang(ang), lin(lin) {}

    static Screw3 zero() { return {}; }

    // Arithmetic
    Screw3 operator+(const Screw3& s) const { return {ang + s.ang, lin + s.lin}; }
    Screw3 operator-(const Screw3& s) const { return {ang - s.ang, lin - s.lin}; }
    Screw3 operator*(double k) const { return {ang * k, lin * k}; }
    Screw3 operator-() const { return {-ang, -lin}; }

    Screw3& operator+=(const Screw3& s) { ang += s.ang; lin += s.lin; return *this; }
    Screw3& operator-=(const Screw3& s) { ang -= s.ang; lin -= s.lin; return *this; }
    Screw3& operator*=(double k) { ang *= k; lin *= k; return *this; }

    // Scale (for v_body * dt)
    Screw3 scaled(double k) const { return {ang * k, lin * k}; }

    // Dot product (for power: wrench · twist)
    double dot(const Screw3& s) const {
        return ang.dot(s.ang) + lin.dot(s.lin);
    }

    // Spatial cross product for motion vectors (twist × twist)
    // [ω₁ × ω₂, ω₁ × v₂ + v₁ × ω₂]
    Screw3 cross_motion(const Screw3& s) const {
        return {
            ang.cross(s.ang),
            ang.cross(s.lin) + lin.cross(s.ang)
        };
    }

    // Spatial cross product for force vectors (twist ×* wrench)
    // [ω × τ + v × f, ω × f]
    Screw3 cross_force(const Screw3& s) const {
        return {
            ang.cross(s.ang) + lin.cross(s.lin),
            ang.cross(s.lin)
        };
    }

    // Transform by pose (rotate both parts)
    Screw3 transform_by(const Pose3& pose) const {
        return {
            pose.transform_vector(ang),
            pose.transform_vector(lin)
        };
    }

    // Inverse transform by pose
    Screw3 inverse_transform_by(const Pose3& pose) const {
        return {
            pose.inverse_transform_vector(ang),
            pose.inverse_transform_vector(lin)
        };
    }

    // Adjoint transform (body -> world) for spatial velocity
    // ω_world = R * ω_body
    // v_world = R * v_body + p × ω_world
    Screw3 adjoint(const Pose3& pose) const {
        Vec3 ang_world = pose.transform_vector(ang);
        Vec3 lin_world = pose.transform_vector(lin) + pose.lin.cross(ang_world);
        return {ang_world, lin_world};
    }

    // Adjoint transform by translation only (kinematic_carry)
    // ω_out = ω
    // v_out = v + ω × arm
    Screw3 adjoint(const Vec3& arm) const {
        return {ang, lin + ang.cross(arm)};
    }

    // Inverse adjoint (world -> body) for spatial velocity
    // ω_body = R^T * ω_world
    // v_body = R^T * (v_world - p × ω_world)
    Screw3 adjoint_inv(const Pose3& pose) const {
        Vec3 ang_body = pose.inverse_transform_vector(ang);
        Vec3 lin_body = pose.inverse_transform_vector(lin - pose.lin.cross(ang));
        return {ang_body, lin_body};
    }

    // Inverse adjoint by translation only
    // ω_out = ω
    // v_out = v - ω × arm
    Screw3 adjoint_inv(const Vec3& arm) const {
        return {ang, lin - ang.cross(arm)};
    }

    // Coadjoint transform (body -> world) for wrench (force vectors)
    // f_world = R * f_body
    // τ_world = R * τ_body + p × f_world
    Screw3 coadjoint(const Pose3& pose) const {
        Vec3 lin_world = pose.transform_vector(lin);
        Vec3 ang_world = pose.transform_vector(ang) + pose.lin.cross(lin_world);
        return {ang_world, lin_world};
    }

    // Coadjoint by translation only (force_carry)
    // f_out = f
    // τ_out = τ + arm × f
    Screw3 coadjoint(const Vec3& arm) const {
        return {ang + arm.cross(lin), lin};
    }

    // Inverse coadjoint (world -> body) for wrench
    // f_body = R^T * f_world
    // τ_body = R^T * (τ_world - p × f_world)
    Screw3 coadjoint_inv(const Pose3& pose) const {
        Vec3 lin_body = pose.inverse_transform_vector(lin);
        Vec3 ang_body = pose.inverse_transform_vector(ang - pose.lin.cross(lin));
        return {ang_body, lin_body};
    }

    // Inverse coadjoint by translation only
    // f_out = f
    // τ_out = τ - arm × f
    Screw3 coadjoint_inv(const Vec3& arm) const {
        return {ang - arm.cross(lin), lin};
    }

    // Convert to Pose3 (exponential map for small motions)
    Pose3 to_pose() const {
        double theta = ang.norm();
        if (theta < 1e-8) {
            return {Quat::identity(), lin};
        }

        Vec3 axis = ang / theta;
        double half = theta * 0.5;
        Quat q = {
            axis.x * std::sin(half),
            axis.y * std::sin(half),
            axis.z * std::sin(half),
            std::cos(half)
        };
        return {q, lin};
    }
};

inline Screw3 operator*(double k, const Screw3& s) { return s * k; }


} // namespace termin
