#pragma once

#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>
#include <nanobind/operators.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/optional.h>

#include <termin/geom/geom.hpp>

namespace nb = nanobind;

namespace termin {

// Helper to create numpy array from Vec3
inline nb::ndarray<nb::numpy, double, nb::shape<3>> vec3_to_numpy(const Vec3& v) {
    double* data = new double[3]{v.x, v.y, v.z};
    nb::capsule owner(data, [](void* p) noexcept { delete[] static_cast<double*>(p); });
    return nb::ndarray<nb::numpy, double, nb::shape<3>>(data, {3}, owner);
}

// Helper to create Vec3 from numpy array
inline Vec3 numpy_to_vec3(nb::ndarray<double, nb::c_contig, nb::device::cpu> arr) {
    double* ptr = arr.data();
    return {ptr[0], ptr[1], ptr[2]};
}

// Helper to create numpy array from Quat
inline nb::ndarray<nb::numpy, double, nb::shape<4>> quat_to_numpy(const Quat& q) {
    double* data = new double[4]{q.x, q.y, q.z, q.w};
    nb::capsule owner(data, [](void* p) noexcept { delete[] static_cast<double*>(p); });
    return nb::ndarray<nb::numpy, double, nb::shape<4>>(data, {4}, owner);
}

// Helper to create Quat from numpy array
inline Quat numpy_to_quat(nb::ndarray<double, nb::c_contig, nb::device::cpu> arr) {
    double* ptr = arr.data();
    return {ptr[0], ptr[1], ptr[2], ptr[3]};
}

// Helper to convert any array-like Python object to Vec3
inline Vec3 py_to_vec3(nb::object obj) {
    if (nb::isinstance<Vec3>(obj)) {
        return nb::cast<Vec3>(obj);
    }
    // Try ndarray
    try {
        auto arr = nb::cast<nb::ndarray<double, nb::c_contig, nb::device::cpu>>(obj);
        double* ptr = arr.data();
        return Vec3{ptr[0], ptr[1], ptr[2]};
    } catch (...) {}
    // Try sequence protocol
    nb::sequence seq = nb::cast<nb::sequence>(obj);
    return Vec3{nb::cast<double>(seq[0]), nb::cast<double>(seq[1]), nb::cast<double>(seq[2])};
}

// Forward declarations for binding functions
void bind_vec3(nb::module_& m);
void bind_vec4(nb::module_& m);
void bind_quat(nb::module_& m);
void bind_mat44(nb::module_& m);
void bind_pose3(nb::module_& m);
void bind_general_pose3(nb::module_& m);
void bind_screw3(nb::module_& m);
void bind_transform(nb::module_& m);
void bind_aabb(nb::module_& m);

} // namespace termin
