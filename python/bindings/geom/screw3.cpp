#include "common.hpp"

namespace termin {

void bind_screw3(nb::module_& m) {
    nb::class_<Screw3>(m, "Screw3")
        .def(nb::init<>())
        .def(nb::init<const Vec3&, const Vec3&>(), nb::arg("ang"), nb::arg("lin"))
        .def("__init__", [](Screw3* self, nb::ndarray<double, nb::c_contig, nb::device::cpu> ang_arr,
                                          nb::ndarray<double, nb::c_contig, nb::device::cpu> lin_arr) {
            new (self) Screw3(numpy_to_vec3(ang_arr), numpy_to_vec3(lin_arr));
        }, nb::arg("ang"), nb::arg("lin"))
        .def_rw("ang", &Screw3::ang)
        .def_rw("lin", &Screw3::lin)
        .def(nb::self + nb::self)
        .def(nb::self - nb::self)
        .def(nb::self * double())
        .def(double() * nb::self)
        .def(-nb::self)
        .def("dot", &Screw3::dot)
        .def("cross_motion", &Screw3::cross_motion)
        .def("cross_force", &Screw3::cross_force)
        .def("transform_by", &Screw3::transform_by)
        .def("inverse_transform_by", &Screw3::inverse_transform_by)
        .def("to_pose", &Screw3::to_pose)
        .def("as_pose3", &Screw3::to_pose)  // alias for compatibility
        .def("scaled", &Screw3::scaled)
        // Adjoint overloads
        .def("adjoint", nb::overload_cast<const Pose3&>(&Screw3::adjoint, nb::const_))
        .def("adjoint", nb::overload_cast<const Vec3&>(&Screw3::adjoint, nb::const_))
        .def("adjoint_inv", nb::overload_cast<const Pose3&>(&Screw3::adjoint_inv, nb::const_))
        .def("adjoint_inv", nb::overload_cast<const Vec3&>(&Screw3::adjoint_inv, nb::const_))
        .def("coadjoint", nb::overload_cast<const Pose3&>(&Screw3::coadjoint, nb::const_))
        .def("coadjoint", nb::overload_cast<const Vec3&>(&Screw3::coadjoint, nb::const_))
        .def("coadjoint_inv", nb::overload_cast<const Pose3&>(&Screw3::coadjoint_inv, nb::const_))
        .def("coadjoint_inv", nb::overload_cast<const Vec3&>(&Screw3::coadjoint_inv, nb::const_))
        // Aliases for compatibility
        .def("kinematic_carry", nb::overload_cast<const Vec3&>(&Screw3::adjoint, nb::const_))
        .def("twist_carry", nb::overload_cast<const Vec3&>(&Screw3::adjoint, nb::const_))
        .def("force_carry", nb::overload_cast<const Vec3&>(&Screw3::coadjoint, nb::const_))
        .def("wrench_carry", nb::overload_cast<const Vec3&>(&Screw3::coadjoint, nb::const_))
        .def_static("zero", &Screw3::zero)
        .def("__repr__", [](const Screw3& s) {
            return "Screw3(ang=Vec3(" + std::to_string(s.ang.x) + ", " +
                   std::to_string(s.ang.y) + ", " + std::to_string(s.ang.z) +
                   "), lin=Vec3(" + std::to_string(s.lin.x) + ", " +
                   std::to_string(s.lin.y) + ", " + std::to_string(s.lin.z) + "))";
        });
}

} // namespace termin
