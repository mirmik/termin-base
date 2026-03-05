#pragma once

#include "vec3.hpp"
#include <algorithm>
#include <array>
#include <cmath>

namespace termin {


/**
 * Axis-Aligned Bounding Box in 3D space.
 */
struct AABB {
    Vec3 min_point;
    Vec3 max_point;

    AABB() : min_point(0, 0, 0), max_point(0, 0, 0) {}
    AABB(const Vec3& min_pt, const Vec3& max_pt) : min_point(min_pt), max_point(max_pt) {}

    // Extend the AABB to include the given point
    void extend(const Vec3& point) {
        min_point.x = std::min(min_point.x, point.x);
        min_point.y = std::min(min_point.y, point.y);
        min_point.z = std::min(min_point.z, point.z);
        max_point.x = std::max(max_point.x, point.x);
        max_point.y = std::max(max_point.y, point.y);
        max_point.z = std::max(max_point.z, point.z);
    }

    // Check if this AABB intersects with another AABB
    bool intersects(const AABB& other) const {
        return max_point.x >= other.min_point.x && other.max_point.x >= min_point.x &&
               max_point.y >= other.min_point.y && other.max_point.y >= min_point.y &&
               max_point.z >= other.min_point.z && other.max_point.z >= min_point.z;
    }

    // Check if this AABB contains a point
    bool contains(const Vec3& point) const {
        return point.x >= min_point.x && point.x <= max_point.x &&
               point.y >= min_point.y && point.y <= max_point.y &&
               point.z >= min_point.z && point.z <= max_point.z;
    }

    // Merge this AABB with another AABB
    AABB merge(const AABB& other) const {
        return AABB(
            Vec3(std::min(min_point.x, other.min_point.x),
                 std::min(min_point.y, other.min_point.y),
                 std::min(min_point.z, other.min_point.z)),
            Vec3(std::max(max_point.x, other.max_point.x),
                 std::max(max_point.y, other.max_point.y),
                 std::max(max_point.z, other.max_point.z))
        );
    }

    // Get center of the AABB
    Vec3 center() const {
        return (min_point + max_point) * 0.5;
    }

    // Get size (extent) of the AABB
    Vec3 size() const {
        return max_point - min_point;
    }

    // Get half size
    Vec3 half_size() const {
        return (max_point - min_point) * 0.5;
    }

    // Project a point onto the AABB (clamp)
    Vec3 project_point(const Vec3& point) const {
        return Vec3(
            std::clamp(point.x, min_point.x, max_point.x),
            std::clamp(point.y, min_point.y, max_point.y),
            std::clamp(point.z, min_point.z, max_point.z)
        );
    }

    // Get the 8 corners of the AABB
    std::array<Vec3, 8> corners() const {
        return {{
            {min_point.x, min_point.y, min_point.z},
            {min_point.x, min_point.y, max_point.z},
            {min_point.x, max_point.y, min_point.z},
            {min_point.x, max_point.y, max_point.z},
            {max_point.x, min_point.y, min_point.z},
            {max_point.x, min_point.y, max_point.z},
            {max_point.x, max_point.y, min_point.z},
            {max_point.x, max_point.y, max_point.z}
        }};
    }

    // Surface area (useful for BVH)
    double surface_area() const {
        Vec3 d = max_point - min_point;
        return 2.0 * (d.x * d.y + d.y * d.z + d.z * d.x);
    }

    // Volume
    double volume() const {
        Vec3 d = max_point - min_point;
        return d.x * d.y * d.z;
    }

    // Create from a set of points
    static AABB from_points(const Vec3* points, size_t count) {
        if (count == 0) return AABB();
        AABB result(points[0], points[0]);
        for (size_t i = 1; i < count; ++i) {
            result.extend(points[i]);
        }
        return result;
    }

    // Transform AABB by pose (returns new AABB that bounds the transformed box)
    template<typename PoseType>
    AABB transformed_by(const PoseType& pose) const {
        auto c = corners();
        Vec3 first = pose.transform_point(c[0]);
        AABB result(first, first);
        for (size_t i = 1; i < 8; ++i) {
            result.extend(pose.transform_point(c[i]));
        }
        return result;
    }
};


} // namespace termin
