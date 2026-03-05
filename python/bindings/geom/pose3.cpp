#include "common.hpp"

namespace termin {

void bind_pose3(nb::module_& m) {
    nb::class_<Pose3>(m, "Pose3")
        .def(nb::init<>())
        .def(nb::init<const Quat&, const Vec3&>())
        // Convenience: Pose3(Vec3) - translation only
        .def("__init__", [](Pose3* self, const Vec3& lin) {
            new (self) Pose3{Quat::identity(), lin};
        })
        .def("__init__", [](Pose3* self,
                           nb::ndarray<double, nb::c_contig, nb::device::cpu> ang_arr,
                           nb::ndarray<double, nb::c_contig, nb::device::cpu> lin_arr) {
            new (self) Pose3(numpy_to_quat(ang_arr), numpy_to_vec3(lin_arr));
        })
        // Python-style constructor with keyword args
        .def("__init__", [](Pose3* self, nb::object ang, nb::object lin) {
            Quat q = Quat::identity();
            Vec3 t = Vec3::zero();

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
            new (self) Pose3{q, t};
        }, nb::arg("ang") = nb::none(), nb::arg("lin") = nb::none())
        .def_prop_rw("ang",
            [](const Pose3& p) { return p.ang; },
            [](Pose3& p, nb::object val) {
                if (nb::isinstance<Quat>(val)) {
                    p.ang = nb::cast<Quat>(val);
                } else {
                    auto arr = nb::cast<nb::ndarray<double, nb::c_contig, nb::device::cpu>>(val);
                    double* ptr = arr.data();
                    p.ang = Quat{ptr[0], ptr[1], ptr[2], ptr[3]};
                }
            })
        .def_prop_rw("lin",
            [](const Pose3& p) { return p.lin; },
            [](Pose3& p, nb::object val) {
                if (nb::isinstance<Vec3>(val)) {
                    p.lin = nb::cast<Vec3>(val);
                } else {
                    auto arr = nb::cast<nb::ndarray<double, nb::c_contig, nb::device::cpu>>(val);
                    double* ptr = arr.data();
                    p.lin = Vec3{ptr[0], ptr[1], ptr[2]};
                }
            })
        .def(nb::self * nb::self)
        .def("__matmul__", [](const Pose3& a, const Pose3& b) { return a * b; })
        .def("inverse", &Pose3::inverse)
        .def("transform_point", nb::overload_cast<const Vec3&>(&Pose3::transform_point, nb::const_))
        .def("transform_point", [](const Pose3& p, nb::object obj) {
            return p.transform_point(py_to_vec3(obj));
        })
        .def("transform_vector", nb::overload_cast<const Vec3&>(&Pose3::transform_vector, nb::const_))
        .def("transform_vector", [](const Pose3& p, nb::object obj) {
            return p.transform_vector(py_to_vec3(obj));
        })
        .def("rotate_point", &Pose3::rotate_point)
        .def("rotate_point", [](const Pose3& p, nb::object obj) {
            return p.rotate_point(py_to_vec3(obj));
        })
        .def("inverse_transform_point", nb::overload_cast<const Vec3&>(&Pose3::inverse_transform_point, nb::const_))
        .def("inverse_transform_point", [](const Pose3& p, nb::object obj) {
            return p.inverse_transform_point(py_to_vec3(obj));
        })
        .def("inverse_transform_vector", nb::overload_cast<const Vec3&>(&Pose3::inverse_transform_vector, nb::const_))
        .def("inverse_transform_vector", [](const Pose3& p, nb::object obj) {
            return p.inverse_transform_vector(py_to_vec3(obj));
        })
        // rotate_vector is an alias for transform_vector (for Pose3 without scale, they are the same)
        .def("rotate_vector", nb::overload_cast<const Vec3&>(&Pose3::transform_vector, nb::const_))
        .def("rotate_vector", [](const Pose3& p, nb::object obj) {
            return p.transform_vector(py_to_vec3(obj));
        })
        .def("inverse_rotate_vector", nb::overload_cast<const Vec3&>(&Pose3::inverse_transform_vector, nb::const_))
        .def("inverse_rotate_vector", [](const Pose3& p, nb::object obj) {
            return p.inverse_transform_vector(py_to_vec3(obj));
        })
        .def("normalized", &Pose3::normalized)
        .def("with_translation", nb::overload_cast<const Vec3&>(&Pose3::with_translation, nb::const_))
        .def("with_rotation", &Pose3::with_rotation)
        .def("rotation_matrix", [](const Pose3& p) {
            double* data = new double[9];
            p.rotation_matrix(data);
            nb::capsule owner(data, [](void* ptr) noexcept { delete[] static_cast<double*>(ptr); });
            size_t shape[2] = {3, 3};
            return nb::ndarray<nb::numpy, double, nb::shape<3, 3>>(data, 2, shape, owner);
        })
        .def_static("identity", &Pose3::identity)
        .def_static("translation", nb::overload_cast<double, double, double>(&Pose3::translation))
        .def_static("rotation", nb::overload_cast<const Vec3&, double>(&Pose3::rotation))
        .def_static("rotation", [](nb::object axis, double angle) {
            return Pose3::rotation(py_to_vec3(axis), angle);
        })
        .def_static("rotate_x", &Pose3::rotate_x)
        .def_static("rotate_y", &Pose3::rotate_y)
        .def_static("rotate_z", &Pose3::rotate_z)
        // Python-style aliases (rotateX instead of rotate_x)
        .def_static("rotateX", &Pose3::rotate_x)
        .def_static("rotateY", &Pose3::rotate_y)
        .def_static("rotateZ", &Pose3::rotate_z)
        // moveX, moveY, moveZ for translation
        .def_static("moveX", [](double d) { return Pose3::translation(d, 0, 0); })
        .def_static("moveY", [](double d) { return Pose3::translation(0, d, 0); })
        .def_static("moveZ", [](double d) { return Pose3::translation(0, 0, d); })
        // looking_at accepting any array-like objects (Vec3, numpy arrays, lists, tuples)
        .def_static("looking_at", [](nb::object eye, nb::object target, nb::object up) {
            Vec3 up_vec = up.is_none() ? Vec3::unit_z() : py_to_vec3(up);
            return Pose3::looking_at(py_to_vec3(eye), py_to_vec3(target), up_vec);
        }, nb::arg("eye"), nb::arg("target"), nb::arg("up") = nb::none())
        .def_static("from_euler", &Pose3::from_euler,
                    nb::arg("roll"), nb::arg("pitch"), nb::arg("yaw"))
        .def("to_euler", nb::overload_cast<>(&Pose3::to_euler, nb::const_))
        .def("to_axis_angle", [](const Pose3& p) {
            Vec3 axis;
            double angle;
            p.to_axis_angle(axis, angle);
            return nb::make_tuple(axis, angle);
        })
        .def("distance", &Pose3::distance)
        .def("copy", &Pose3::copy)
        .def("as_matrix", [](const Pose3& p) {
            double* data = new double[16];
            double m[16];
            p.as_matrix(m);
            for (int i = 0; i < 4; i++)
                for (int j = 0; j < 4; j++)
                    data[i * 4 + j] = m[j * 4 + i];  // column-major to row-major
            nb::capsule owner(data, [](void* ptr) noexcept { delete[] static_cast<double*>(ptr); });
            size_t shape[2] = {4, 4};
            return nb::ndarray<nb::numpy, double, nb::shape<4, 4>>(data, 2, shape, owner);
        })
        .def("as_mat44", [](const Pose3& p) {
            Mat44 mat;
            p.as_matrix(mat.data);
            return mat;
        })
        .def("as_matrix34", [](const Pose3& p) {
            double* data = new double[12];
            double rot[9];
            p.rotation_matrix(rot);
            for (int i = 0; i < 3; i++)
                for (int j = 0; j < 3; j++)
                    data[i * 4 + j] = rot[i * 3 + j];
            data[3] = p.lin.x;
            data[7] = p.lin.y;
            data[11] = p.lin.z;
            nb::capsule owner(data, [](void* ptr) noexcept { delete[] static_cast<double*>(ptr); });
            size_t shape[2] = {3, 4};
            return nb::ndarray<nb::numpy, double, nb::shape<3, 4>>(data, 2, shape, owner);
        })
        .def("compose", [](const Pose3& a, const Pose3& b) { return a * b; })
        // x, y, z property shortcuts for translation
        .def_prop_rw("x",
            [](const Pose3& p) { return p.lin.x; },
            [](Pose3& p, double v) { p.lin.x = v; })
        .def_prop_rw("y",
            [](const Pose3& p) { return p.lin.y; },
            [](Pose3& p, double v) { p.lin.y = v; })
        .def_prop_rw("z",
            [](const Pose3& p) { return p.lin.z; },
            [](Pose3& p, double v) { p.lin.z = v; })
        // Static translation methods (aliases)
        .def_static("right", [](double d) { return Pose3::translation(d, 0, 0); })
        .def_static("forward", [](double d) { return Pose3::translation(0, d, 0); })
        .def_static("up", [](double d) { return Pose3::translation(0, 0, d); })
        // Static from_axis_angle
        .def_static("from_axis_angle", [](const Vec3& axis, double angle) {
            return Pose3::rotation(axis, angle);
        })
        // Static lerp
        .def_static("lerp", [](const Pose3& a, const Pose3& b, double t) {
            return lerp(a, b, t);
        })
        .def("to_general_pose3", [](const Pose3& p, const Vec3& scale) {
            return GeneralPose3(p.ang, p.lin, scale);
        }, nb::arg("scale") = Vec3{1.0, 1.0, 1.0})
        .def("__repr__", [](const Pose3& p) {
            return "Pose3(ang=Quat(" + std::to_string(p.ang.x) + ", " +
                   std::to_string(p.ang.y) + ", " + std::to_string(p.ang.z) + ", " +
                   std::to_string(p.ang.w) + "), lin=Vec3(" +
                   std::to_string(p.lin.x) + ", " + std::to_string(p.lin.y) + ", " +
                   std::to_string(p.lin.z) + "))";
        });

    m.def("lerp",
          static_cast<Pose3 (*)(const Pose3&, const Pose3&, double)>(&lerp),
          "Linear interpolation between poses");
}

} // namespace termin
