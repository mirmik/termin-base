#include "common.hpp"
#include <nanobind/stl/pair.h>

namespace termin {

void bind_mat44(nb::module_& m) {
    nb::class_<Mat44>(m, "Mat44")
        .def(nb::init<>())
        .def("__call__", [](const Mat44& m, int col, int row) { return m(col, row); })
        .def("__getitem__", [](const Mat44& m, std::pair<int, int> idx) {
            return m(idx.first, idx.second);
        })
        .def("__setitem__", [](Mat44& m, std::pair<int, int> idx, double val) {
            m(idx.first, idx.second) = val;
        })
        .def(nb::self * nb::self)
        .def("__matmul__", [](const Mat44& a, const Mat44& b) { return a * b; })
        .def("__matmul__", [](const Mat44& m, const Vec3& v) { return m.transform_point(v); })
        // Mat44 @ numpy 4-vector -> numpy 4-vector (homogeneous transform)
        .def("__matmul__", [](const Mat44& m, nb::ndarray<nb::numpy, double, nb::shape<4>> v) {
            double x = m(0, 0) * v(0) + m(1, 0) * v(1) + m(2, 0) * v(2) + m(3, 0) * v(3);
            double y = m(0, 1) * v(0) + m(1, 1) * v(1) + m(2, 1) * v(2) + m(3, 1) * v(3);
            double z = m(0, 2) * v(0) + m(1, 2) * v(1) + m(2, 2) * v(2) + m(3, 2) * v(3);
            double w = m(0, 3) * v(0) + m(1, 3) * v(1) + m(2, 3) * v(2) + m(3, 3) * v(3);
            double* data = new double[4]{x, y, z, w};
            nb::capsule owner(data, [](void* p) noexcept { delete[] static_cast<double*>(p); });
            return nb::ndarray<nb::numpy, double, nb::shape<4>>(data, {4}, owner);
        })
        .def("__matmul__", [](const Mat44& m, nb::ndarray<nb::numpy, float, nb::shape<4>> v) {
            float x = static_cast<float>(m(0, 0) * v(0) + m(1, 0) * v(1) + m(2, 0) * v(2) + m(3, 0) * v(3));
            float y = static_cast<float>(m(0, 1) * v(0) + m(1, 1) * v(1) + m(2, 1) * v(2) + m(3, 1) * v(3));
            float z = static_cast<float>(m(0, 2) * v(0) + m(1, 2) * v(1) + m(2, 2) * v(2) + m(3, 2) * v(3));
            float w = static_cast<float>(m(0, 3) * v(0) + m(1, 3) * v(1) + m(2, 3) * v(2) + m(3, 3) * v(3));
            float* data = new float[4]{x, y, z, w};
            nb::capsule owner(data, [](void* p) noexcept { delete[] static_cast<float*>(p); });
            return nb::ndarray<nb::numpy, float, nb::shape<4>>(data, {4}, owner);
        })
        .def("transform_point", &Mat44::transform_point)
        .def("transform_direction", &Mat44::transform_direction)
        .def("transposed", &Mat44::transposed)
        .def("inverse", &Mat44::inverse)
        .def("get_translation", &Mat44::get_translation)
        .def("get_scale", &Mat44::get_scale)
        .def("with_translation", nb::overload_cast<const Vec3&>(&Mat44::with_translation, nb::const_))
        .def("with_translation", nb::overload_cast<double, double, double>(&Mat44::with_translation, nb::const_))
        .def_static("identity", &Mat44::identity)
        .def_static("zero", &Mat44::zero)
        .def_static("translation", nb::overload_cast<const Vec3&>(&Mat44::translation))
        .def_static("translation", nb::overload_cast<double, double, double>(&Mat44::translation))
        .def_static("scale", nb::overload_cast<const Vec3&>(&Mat44::scale))
        .def_static("scale", nb::overload_cast<double>(&Mat44::scale))
        .def_static("rotation", &Mat44::rotation)
        .def_static("rotation_axis_angle", &Mat44::rotation_axis_angle)
        .def_static("perspective", &Mat44::perspective,
            nb::arg("fov_y"), nb::arg("aspect"), nb::arg("near"), nb::arg("far"),
            "Perspective projection (Y-forward, Z-up)")
        .def_static("orthographic", &Mat44::orthographic,
            nb::arg("left"), nb::arg("right"), nb::arg("bottom"), nb::arg("top"),
            nb::arg("near"), nb::arg("far"),
            "Orthographic projection (Y-forward, Z-up)")
        .def_static("look_at", &Mat44::look_at,
            nb::arg("eye"), nb::arg("target"), nb::arg("up") = Vec3::unit_z(),
            "Look-at view matrix (Y-forward, Z-up)")
        .def_static("compose", &Mat44::compose,
            nb::arg("translation"), nb::arg("rotation"), nb::arg("scale"),
            "Compose TRS matrix")
        .def("to_numpy", [](const Mat44& mat) {
            double* data = new double[16];
            // Column-major to row-major for numpy
            for (int row = 0; row < 4; ++row)
                for (int col = 0; col < 4; ++col)
                    data[row * 4 + col] = mat(col, row);
            nb::capsule owner(data, [](void* p) noexcept { delete[] static_cast<double*>(p); });
            size_t shape[2] = {4, 4};
            return nb::ndarray<nb::numpy, double, nb::shape<4, 4>>(data, 2, shape, owner);
        })
        .def("to_numpy_f32", [](const Mat44& mat) {
            float* data = new float[16];
            // Column-major to row-major for numpy
            for (int row = 0; row < 4; ++row)
                for (int col = 0; col < 4; ++col)
                    data[row * 4 + col] = static_cast<float>(mat(col, row));
            nb::capsule owner(data, [](void* p) noexcept { delete[] static_cast<float*>(p); });
            size_t shape[2] = {4, 4};
            return nb::ndarray<nb::numpy, float, nb::shape<4, 4>>(data, 2, shape, owner);
        })
        .def("__repr__", [](const Mat44& m) {
            return "<Mat44>";
        })
        .def("to_float", &Mat44::to_float);

    // Mat44f (float version)
    nb::class_<Mat44f>(m, "Mat44f")
        .def(nb::init<>())
        .def("__call__", [](const Mat44f& m, int col, int row) { return m(col, row); })
        .def("__getitem__", [](const Mat44f& m, std::pair<int, int> idx) {
            return m(idx.first, idx.second);
        })
        .def("__setitem__", [](Mat44f& m, std::pair<int, int> idx, float val) {
            m(idx.first, idx.second) = val;
        })
        .def(nb::self * nb::self)
        .def("__matmul__", [](const Mat44f& a, const Mat44f& b) { return a * b; })
        .def("__matmul__", [](const Mat44f& m, const Vec3& v) { return m.transform_point(v); })
        .def("transform_point", &Mat44f::transform_point)
        .def("transform_direction", &Mat44f::transform_direction)
        .def("transposed", &Mat44f::transposed)
        .def("inverse", &Mat44f::inverse)
        .def("get_translation", &Mat44f::get_translation)
        .def("get_scale", &Mat44f::get_scale)
        .def("with_translation", nb::overload_cast<const Vec3&>(&Mat44f::with_translation, nb::const_))
        .def("with_translation", nb::overload_cast<float, float, float>(&Mat44f::with_translation, nb::const_))
        .def_static("identity", &Mat44f::identity)
        .def_static("zero", &Mat44f::zero)
        .def_static("translation", nb::overload_cast<const Vec3&>(&Mat44f::translation))
        .def_static("translation", nb::overload_cast<float, float, float>(&Mat44f::translation))
        .def_static("scale", nb::overload_cast<const Vec3&>(&Mat44f::scale))
        .def_static("scale", nb::overload_cast<float>(&Mat44f::scale))
        .def_static("rotation", &Mat44f::rotation)
        .def_static("rotation_axis_angle", &Mat44f::rotation_axis_angle)
        .def_static("perspective", &Mat44f::perspective,
            nb::arg("fov_y"), nb::arg("aspect"), nb::arg("near"), nb::arg("far"))
        .def_static("orthographic", &Mat44f::orthographic,
            nb::arg("left"), nb::arg("right"), nb::arg("bottom"), nb::arg("top"),
            nb::arg("near"), nb::arg("far"))
        .def_static("look_at", &Mat44f::look_at,
            nb::arg("eye"), nb::arg("target"), nb::arg("up") = Vec3::unit_z())
        .def_static("compose", &Mat44f::compose,
            nb::arg("translation"), nb::arg("rotation"), nb::arg("scale"))
        .def("to_numpy", [](const Mat44f& mat) {
            float* data = new float[16];
            // Column-major to row-major for numpy
            for (int row = 0; row < 4; ++row)
                for (int col = 0; col < 4; ++col)
                    data[row * 4 + col] = mat(col, row);
            nb::capsule owner(data, [](void* p) noexcept { delete[] static_cast<float*>(p); });
            size_t shape[2] = {4, 4};
            return nb::ndarray<nb::numpy, float, nb::shape<4, 4>>(data, 2, shape, owner);
        })
        .def("__repr__", [](const Mat44f& m) {
            return "<Mat44f>";
        });
}

} // namespace termin
