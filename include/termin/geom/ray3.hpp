#pragma once

#include "vec3.hpp"
#include <cmath>

namespace termin {


/**
 * Луч в 3D пространстве.
 * origin — начало луча
 * direction — нормализованное направление
 */
struct Ray3 {
    Vec3 origin;
    Vec3 direction;  // всегда нормализовано

    Ray3() : origin(0, 0, 0), direction(0, 0, 1) {}

    Ray3(const Vec3& origin, const Vec3& dir)
        : origin(origin)
    {
        double n = dir.norm();
        direction = (n > 1e-10) ? dir / n : Vec3(0, 0, 1);
    }

    /**
     * Точка на луче при параметре t: P(t) = origin + direction * t
     */
    Vec3 point_at(double t) const {
        return origin + direction * t;
    }
};


} // namespace termin
