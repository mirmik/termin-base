#include "common.hpp"

namespace termin {

void bind_quat(nb::module_& m) {
    nb::class_<Quat>(m, "Quat")
        .def(nb::init<>())
        .def(nb::init<double, double, double, double>())
        .def("__init__", [](Quat* self, nb::ndarray<double, nb::c_contig, nb::device::cpu> arr) {
            double* ptr = arr.data();
            new (self) Quat{ptr[0], ptr[1], ptr[2], ptr[3]};
        })
        .def_rw("x", &Quat::x)
        .def_rw("y", &Quat::y)
        .def_rw("z", &Quat::z)
        .def_rw("w", &Quat::w)
        .def("__getitem__", [](const Quat& q, int i) {
            if (i == 0) return q.x;
            if (i == 1) return q.y;
            if (i == 2) return q.z;
            if (i == 3) return q.w;
            throw nb::index_error("Quat index out of range");
        })
        .def("__setitem__", [](Quat& q, int i, double val) {
            if (i == 0) q.x = val;
            else if (i == 1) q.y = val;
            else if (i == 2) q.z = val;
            else if (i == 3) q.w = val;
            else throw nb::index_error("Quat index out of range");
        })
        .def("__len__", [](const Quat&) { return 4; })
        .def("__iter__", [](const Quat& q) {
            return nb::iter(nb::make_tuple(q.x, q.y, q.z, q.w));
        })
        .def(nb::self * nb::self)
        .def("conjugate", &Quat::conjugate)
        .def("inverse", &Quat::inverse)
        .def("norm", &Quat::norm)
        .def("normalized", &Quat::normalized)
        .def("rotate", &Quat::rotate)
        .def("inverse_rotate", &Quat::inverse_rotate)
        .def_static("identity", &Quat::identity)
        .def_static("from_axis_angle", &Quat::from_axis_angle)
        .def_static("look_rotation", &Quat::look_rotation,
            nb::arg("forward"), nb::arg("up") = Vec3::unit_z(),
            "Create quaternion looking in direction (Forward=+Y, Up=+Z)")
        .def_static("slerp", &Quat::slerp,
            nb::arg("a"), nb::arg("b"), nb::arg("t"),
            "Spherical linear interpolation between quaternions")
        .def("to_numpy", &quat_to_numpy)
        .def("tolist", [](const Quat& q) {
            nb::list lst;
            lst.append(q.x);
            lst.append(q.y);
            lst.append(q.z);
            lst.append(q.w);
            return lst;
        })
        .def("copy", [](const Quat& q) { return q; })
        .def("__repr__", [](const Quat& q) {
            return "Quat(" + std::to_string(q.x) + ", " +
                   std::to_string(q.y) + ", " + std::to_string(q.z) + ", " +
                   std::to_string(q.w) + ")";
        });

    m.def("slerp", &slerp, "Spherical linear interpolation between quaternions");
}

} // namespace termin
