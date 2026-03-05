#include "common.hpp"

namespace termin {

void bind_vec4(nb::module_& m) {
    nb::class_<Vec4>(m, "Vec4")
        .def(nb::init<>())
        .def(nb::init<double, double, double, double>())
        .def_rw("x", &Vec4::x)
        .def_rw("y", &Vec4::y)
        .def_rw("z", &Vec4::z)
        .def_rw("w", &Vec4::w)
        .def("__getitem__", [](const Vec4& v, int i) { return v[i]; })
        .def("__setitem__", [](Vec4& v, int i, double val) { v[i] = val; })
        .def("__len__", [](const Vec4&) { return 4; })
        .def("__iter__", [](const Vec4& v) {
            return nb::iter(nb::make_tuple(v.x, v.y, v.z, v.w));
        })
        .def(nb::self + nb::self)
        .def(nb::self - nb::self)
        .def(nb::self * double())
        .def(double() * nb::self)
        .def(nb::self / double())
        .def(-nb::self)
        .def("dot", &Vec4::dot)
        .def("norm", &Vec4::norm)
        .def("norm_squared", &Vec4::norm_squared)
        .def("normalized", &Vec4::normalized)
        .def_static("zero", &Vec4::zero)
        .def_static("unit_x", &Vec4::unit_x)
        .def_static("unit_y", &Vec4::unit_y)
        .def_static("unit_z", &Vec4::unit_z)
        .def_static("unit_w", &Vec4::unit_w)
        .def("tolist", [](const Vec4& v) {
            nb::list lst;
            lst.append(v.x);
            lst.append(v.y);
            lst.append(v.z);
            lst.append(v.w);
            return lst;
        })
        .def("copy", [](const Vec4& v) { return v; })
        .def("__eq__", &Vec4::operator==)
        .def("__ne__", &Vec4::operator!=)
        .def("__repr__", [](const Vec4& v) {
            return "Vec4(" + std::to_string(v.x) + ", " +
                   std::to_string(v.y) + ", " + std::to_string(v.z) + ", " +
                   std::to_string(v.w) + ")";
        });
}

} // namespace termin
