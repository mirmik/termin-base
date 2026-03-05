#include "common.hpp"

namespace termin {

void bind_general_pose3(nb::module_& m) {
    nb::class_<GeneralPose3>(m, "GeneralPose3")
        .def(nb::init<>())
        .def(nb::init<const Quat&, const Vec3&, const Vec3&>(),
             nb::arg("ang"), nb::arg("lin"), nb::arg("scale") = Vec3{1.0, 1.0, 1.0})
        .def("__init__", [](GeneralPose3* self,
                           nb::ndarray<double, nb::c_contig, nb::device::cpu> ang_arr,
                           nb::ndarray<double, nb::c_contig, nb::device::cpu> lin_arr,
                           nb::ndarray<double, nb::c_contig, nb::device::cpu> scale_arr) {
            new (self) GeneralPose3(numpy_to_quat(ang_arr), numpy_to_vec3(lin_arr), numpy_to_vec3(scale_arr));
        })
        // Python-style constructor with keyword args (matching Python GeneralPose3)
        .def("__init__", [](GeneralPose3* self, nb::object ang, nb::object lin, nb::object scale) {
            Quat q = Quat::identity();
            Vec3 t = Vec3::zero();
            Vec3 s{1.0, 1.0, 1.0};

            if (!ang.is_none()) {
                if (nb::isinstance<Quat>(ang)) {
                    q = nb::cast<Quat>(ang);
                } else {
                    auto arr = nb::cast<nb::ndarray<double, nb::c_contig, nb::device::cpu>>(ang);
                    double* ptr = arr.data();
                    q = Quat{ptr[0], ptr[1], ptr[2], ptr[3]};
                }
            }
            if (!lin.is_none()) {
                if (nb::isinstance<Vec3>(lin)) {
                    t = nb::cast<Vec3>(lin);
                } else {
                    auto arr = nb::cast<nb::ndarray<double, nb::c_contig, nb::device::cpu>>(lin);
                    double* ptr = arr.data();
                    t = Vec3{ptr[0], ptr[1], ptr[2]};
                }
            }
            if (!scale.is_none()) {
                if (nb::isinstance<Vec3>(scale)) {
                    s = nb::cast<Vec3>(scale);
                } else {
                    auto arr = nb::cast<nb::ndarray<double, nb::c_contig, nb::device::cpu>>(scale);
                    double* ptr = arr.data();
                    s = Vec3{ptr[0], ptr[1], ptr[2]};
                }
            }
            new (self) GeneralPose3{q, t, s};
        }, nb::arg("ang") = nb::none(), nb::arg("lin") = nb::none(), nb::arg("scale") = nb::none())
        .def_prop_rw("ang",
            [](const GeneralPose3& p) { return p.ang; },
            [](GeneralPose3& p, nb::object val) {
                if (nb::isinstance<Quat>(val)) {
                    p.ang = nb::cast<Quat>(val);
                } else {
                    auto arr = nb::cast<nb::ndarray<double, nb::c_contig, nb::device::cpu>>(val);
                    double* ptr = arr.data();
                    p.ang = Quat{ptr[0], ptr[1], ptr[2], ptr[3]};
                }
            })
        .def_prop_rw("lin",
            [](const GeneralPose3& p) { return p.lin; },
            [](GeneralPose3& p, nb::object val) {
                if (nb::isinstance<Vec3>(val)) {
                    p.lin = nb::cast<Vec3>(val);
                } else {
                    auto arr = nb::cast<nb::ndarray<double, nb::c_contig, nb::device::cpu>>(val);
                    double* ptr = arr.data();
                    p.lin = Vec3{ptr[0], ptr[1], ptr[2]};
                }
            })
        .def_prop_rw("scale",
            [](const GeneralPose3& p) { return p.scale; },
            [](GeneralPose3& p, nb::object val) {
                if (nb::isinstance<Vec3>(val)) {
                    p.scale = nb::cast<Vec3>(val);
                } else {
                    auto arr = nb::cast<nb::ndarray<double, nb::c_contig, nb::device::cpu>>(val);
                    double* ptr = arr.data();
                    p.scale = Vec3{ptr[0], ptr[1], ptr[2]};
                }
            })
        .def(nb::self * nb::self)
        .def("__matmul__", [](const GeneralPose3& a, const GeneralPose3& b) { return a * b; })
        .def("inverse", &GeneralPose3::inverse)
        .def("transform_point", nb::overload_cast<const Vec3&>(&GeneralPose3::transform_point, nb::const_))
        .def("transform_point", [](const GeneralPose3& p, nb::object obj) {
            return p.transform_point(py_to_vec3(obj));
        })
        .def("transform_vector", nb::overload_cast<const Vec3&>(&GeneralPose3::transform_vector, nb::const_))
        .def("transform_vector", [](const GeneralPose3& p, nb::object obj) {
            return p.transform_vector(py_to_vec3(obj));
        })
        .def("rotate_point", nb::overload_cast<const Vec3&>(&GeneralPose3::rotate_point, nb::const_))
        .def("rotate_point", [](const GeneralPose3& p, nb::object obj) {
            return p.rotate_point(py_to_vec3(obj));
        })
        .def("inverse_transform_point", nb::overload_cast<const Vec3&>(&GeneralPose3::inverse_transform_point, nb::const_))
        .def("inverse_transform_point", [](const GeneralPose3& p, nb::object obj) {
            return p.inverse_transform_point(py_to_vec3(obj));
        })
        .def("inverse_transform_vector", nb::overload_cast<const Vec3&>(&GeneralPose3::inverse_transform_vector, nb::const_))
        .def("inverse_transform_vector", [](const GeneralPose3& p, nb::object obj) {
            return p.inverse_transform_vector(py_to_vec3(obj));
        })
        .def("normalized", &GeneralPose3::normalized)
        .def("with_translation", nb::overload_cast<const Vec3&>(&GeneralPose3::with_translation, nb::const_))
        .def("with_rotation", &GeneralPose3::with_rotation)
        .def("with_scale", nb::overload_cast<const Vec3&>(&GeneralPose3::with_scale, nb::const_))
        .def("to_pose3", &GeneralPose3::to_pose3)
        .def("rotation_matrix", [](const GeneralPose3& p) {
            double* data = new double[9];
            p.rotation_matrix(data);
            nb::capsule owner(data, [](void* ptr) noexcept { delete[] static_cast<double*>(ptr); });
            size_t shape[2] = {3, 3};
            return nb::ndarray<nb::numpy, double, nb::shape<3, 3>>(data, 2, shape, owner);
        })
        .def("as_matrix", [](const GeneralPose3& p) {
            double* data = new double[16];
            double m_arr[16];
            p.matrix4(m_arr);  // Column-major
            // Column-major to row-major for numpy
            for (int row = 0; row < 4; row++)
                for (int col = 0; col < 4; col++)
                    data[row * 4 + col] = m_arr[col * 4 + row];
            nb::capsule owner(data, [](void* ptr) noexcept { delete[] static_cast<double*>(ptr); });
            size_t shape[2] = {4, 4};
            return nb::ndarray<nb::numpy, double, nb::shape<4, 4>>(data, 2, shape, owner);
        })
        .def("as_mat44", [](const GeneralPose3& p) {
            // Build column-major 4x4 TRS matrix (same convention as Pose3::as_matrix)
            double rot[9];
            p.rotation_matrix(rot);
            Mat44 m;
            // Column 0
            m(0, 0) = rot[0] * p.scale.x;
            m(0, 1) = rot[3] * p.scale.x;
            m(0, 2) = rot[6] * p.scale.x;
            m(0, 3) = 0;
            // Column 1
            m(1, 0) = rot[1] * p.scale.y;
            m(1, 1) = rot[4] * p.scale.y;
            m(1, 2) = rot[7] * p.scale.y;
            m(1, 3) = 0;
            // Column 2
            m(2, 0) = rot[2] * p.scale.z;
            m(2, 1) = rot[5] * p.scale.z;
            m(2, 2) = rot[8] * p.scale.z;
            m(2, 3) = 0;
            // Column 3
            m(3, 0) = p.lin.x;
            m(3, 1) = p.lin.y;
            m(3, 2) = p.lin.z;
            m(3, 3) = 1;
            return m;
        })
        .def("as_matrix34", [](const GeneralPose3& p) {
            double* data = new double[12];
            double m_arr[12];
            p.matrix34(m_arr);  // Column-major (4 cols x 3 rows)
            // Column-major to row-major for numpy (3 rows x 4 cols)
            for (int row = 0; row < 3; row++)
                for (int col = 0; col < 4; col++)
                    data[row * 4 + col] = m_arr[col * 3 + row];
            nb::capsule owner(data, [](void* ptr) noexcept { delete[] static_cast<double*>(ptr); });
            size_t shape[2] = {3, 4};
            return nb::ndarray<nb::numpy, double, nb::shape<3, 4>>(data, 2, shape, owner);
        })
        .def("inverse_matrix", [](const GeneralPose3& p) {
            double* data = new double[16];
            double m_arr[16];
            p.inverse_matrix4(m_arr);  // Column-major
            // Column-major to row-major for numpy
            for (int row = 0; row < 4; row++)
                for (int col = 0; col < 4; col++)
                    data[row * 4 + col] = m_arr[col * 4 + row];
            nb::capsule owner(data, [](void* ptr) noexcept { delete[] static_cast<double*>(ptr); });
            size_t shape[2] = {4, 4};
            return nb::ndarray<nb::numpy, double, nb::shape<4, 4>>(data, 2, shape, owner);
        })
        .def_static("identity", &GeneralPose3::identity)
        .def_static("translation", nb::overload_cast<double, double, double>(&GeneralPose3::translation))
        .def_static("translation", nb::overload_cast<const Vec3&>(&GeneralPose3::translation))
        .def_static("rotation", &GeneralPose3::rotation)
        .def_static("scaling", nb::overload_cast<double>(&GeneralPose3::scaling))
        .def_static("scaling", nb::overload_cast<double, double, double>(&GeneralPose3::scaling))
        .def_static("rotate_x", &GeneralPose3::rotate_x)
        .def_static("rotate_y", &GeneralPose3::rotate_y)
        .def_static("rotate_z", &GeneralPose3::rotate_z)
        // Python-style aliases (rotateX instead of rotate_x)
        .def_static("rotateX", &GeneralPose3::rotate_x)
        .def_static("rotateY", &GeneralPose3::rotate_y)
        .def_static("rotateZ", &GeneralPose3::rotate_z)
        .def("copy", [](const GeneralPose3& p) { return p; })
        .def_static("move", &GeneralPose3::move)
        .def_static("move_x", &GeneralPose3::move_x)
        .def_static("move_y", &GeneralPose3::move_y)
        .def_static("move_z", &GeneralPose3::move_z)
        .def_static("right", &GeneralPose3::right)
        .def_static("forward", &GeneralPose3::forward)
        .def_static("up", &GeneralPose3::up)
        .def_static("looking_at", nb::overload_cast<const Vec3&, const Vec3&, const Vec3&>(&GeneralPose3::looking_at),
                    nb::arg("eye"), nb::arg("target"), nb::arg("up_vec") = Vec3{0.0, 0.0, 1.0})
        .def_static("looking_at", [](nb::ndarray<double, nb::c_contig, nb::device::cpu> eye,
                                     nb::ndarray<double, nb::c_contig, nb::device::cpu> target,
                                     nb::object up_obj) {
            Vec3 up{0.0, 0.0, 1.0};
            if (!up_obj.is_none()) {
                if (nb::isinstance<Vec3>(up_obj)) {
                    up = nb::cast<Vec3>(up_obj);
                } else {
                    auto up_arr = nb::cast<nb::ndarray<double, nb::c_contig, nb::device::cpu>>(up_obj);
                    double* ptr = up_arr.data();
                    up = Vec3{ptr[0], ptr[1], ptr[2]};
                }
            }
            double* eye_ptr = eye.data();
            double* target_ptr = target.data();
            return GeneralPose3::looking_at(
                Vec3{eye_ptr[0], eye_ptr[1], eye_ptr[2]},
                Vec3{target_ptr[0], target_ptr[1], target_ptr[2]},
                up
            );
        }, nb::arg("eye"), nb::arg("target"), nb::arg("up_vec") = nb::none())
        .def_static("lerp",
                    static_cast<GeneralPose3 (*)(const GeneralPose3&, const GeneralPose3&, double)>(&lerp),
                    "Linear interpolation between GeneralPose3 (with scale)")
        .def_static("from_matrix", [](nb::ndarray<double, nb::c_contig, nb::device::cpu> mat) {
            double* buf = mat.data();
            // Extract translation from 4th column
            Vec3 lin{buf[0 * 4 + 3], buf[1 * 4 + 3], buf[2 * 4 + 3]};

            // Extract column vectors of upper-left 3x3 for scale
            Vec3 col0{buf[0 * 4 + 0], buf[1 * 4 + 0], buf[2 * 4 + 0]};
            Vec3 col1{buf[0 * 4 + 1], buf[1 * 4 + 1], buf[2 * 4 + 1]};
            Vec3 col2{buf[0 * 4 + 2], buf[1 * 4 + 2], buf[2 * 4 + 2]};

            // Scale is the length of each column
            Vec3 scale{col0.norm(), col1.norm(), col2.norm()};

            // Build rotation matrix by dividing out scale
            double rot[9];
            if (scale.x > 1e-10) {
                rot[0] = col0.x / scale.x; rot[3] = col0.y / scale.x; rot[6] = col0.z / scale.x;
            } else {
                rot[0] = 1; rot[3] = 0; rot[6] = 0;
            }
            if (scale.y > 1e-10) {
                rot[1] = col1.x / scale.y; rot[4] = col1.y / scale.y; rot[7] = col1.z / scale.y;
            } else {
                rot[1] = 0; rot[4] = 1; rot[7] = 0;
            }
            if (scale.z > 1e-10) {
                rot[2] = col2.x / scale.z; rot[5] = col2.y / scale.z; rot[8] = col2.z / scale.z;
            } else {
                rot[2] = 0; rot[5] = 0; rot[8] = 1;
            }

            // Convert rotation matrix to quaternion
            Quat q = Quat::from_rotation_matrix(rot);

            return GeneralPose3(q, lin, scale);
        })
        .def("__repr__", [](const GeneralPose3& p) {
            return "GeneralPose3(ang=Quat(" + std::to_string(p.ang.x) + ", " +
                   std::to_string(p.ang.y) + ", " + std::to_string(p.ang.z) + ", " +
                   std::to_string(p.ang.w) + "), lin=Vec3(" +
                   std::to_string(p.lin.x) + ", " + std::to_string(p.lin.y) + ", " +
                   std::to_string(p.lin.z) + "), scale=Vec3(" +
                   std::to_string(p.scale.x) + ", " + std::to_string(p.scale.y) + ", " +
                   std::to_string(p.scale.z) + "))";
        });

    m.def("lerp_general_pose3",
          static_cast<GeneralPose3 (*)(const GeneralPose3&, const GeneralPose3&, double)>(&lerp),
          "Linear interpolation between GeneralPose3 (with scale)");
}

} // namespace termin
