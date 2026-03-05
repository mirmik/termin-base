#include "common.hpp"

namespace termin {

void bind_aabb(nb::module_& m) {
    nb::class_<AABB>(m, "AABB")
        .def(nb::init<>())
        .def(nb::init<const Vec3&, const Vec3&>())
        .def("__init__", [](AABB* self,
                           nb::ndarray<double, nb::c_contig, nb::device::cpu> min_arr,
                           nb::ndarray<double, nb::c_contig, nb::device::cpu> max_arr) {
            new (self) AABB(numpy_to_vec3(min_arr), numpy_to_vec3(max_arr));
        })
        .def_rw("min_point", &AABB::min_point)
        .def_rw("max_point", &AABB::max_point)
        .def("extend", &AABB::extend)
        .def("intersects", &AABB::intersects)
        .def("contains", &AABB::contains)
        .def("merge", &AABB::merge)
        .def("center", &AABB::center)
        .def("size", &AABB::size)
        .def("half_size", &AABB::half_size)
        .def("project_point", &AABB::project_point)
        .def("surface_area", &AABB::surface_area)
        .def("volume", &AABB::volume)
        .def("corners", [](const AABB& aabb) {
            auto corners = aabb.corners();
            double* data = new double[24];  // 8 corners * 3 coords
            for (size_t i = 0; i < 8; ++i) {
                data[i * 3 + 0] = corners[i].x;
                data[i * 3 + 1] = corners[i].y;
                data[i * 3 + 2] = corners[i].z;
            }
            nb::capsule owner(data, [](void* p) noexcept { delete[] static_cast<double*>(p); });
            size_t shape[2] = {8, 3};
            return nb::ndarray<nb::numpy, double, nb::shape<8, 3>>(data, 2, shape, owner);
        })
        .def("get_corners_homogeneous", [](const AABB& aabb) {
            auto corners = aabb.corners();
            double* data = new double[32];  // 8 corners * 4 coords
            for (size_t i = 0; i < 8; ++i) {
                data[i * 4 + 0] = corners[i].x;
                data[i * 4 + 1] = corners[i].y;
                data[i * 4 + 2] = corners[i].z;
                data[i * 4 + 3] = 1.0;
            }
            nb::capsule owner(data, [](void* p) noexcept { delete[] static_cast<double*>(p); });
            size_t shape[2] = {8, 4};
            return nb::ndarray<nb::numpy, double, nb::shape<8, 4>>(data, 2, shape, owner);
        })
        .def_static("from_points", [](nb::ndarray<double, nb::c_contig, nb::device::cpu> points) {
            size_t n = points.shape(0);
            if (n == 0) {
                return AABB();
            }
            double* ptr = points.data();
            size_t stride = points.shape(1);
            Vec3 min_pt{ptr[0], ptr[1], ptr[2]};
            Vec3 max_pt = min_pt;
            for (size_t i = 1; i < n; ++i) {
                Vec3 p{ptr[i * stride + 0], ptr[i * stride + 1], ptr[i * stride + 2]};
                min_pt.x = std::min(min_pt.x, p.x);
                min_pt.y = std::min(min_pt.y, p.y);
                min_pt.z = std::min(min_pt.z, p.z);
                max_pt.x = std::max(max_pt.x, p.x);
                max_pt.y = std::max(max_pt.y, p.y);
                max_pt.z = std::max(max_pt.z, p.z);
            }
            return AABB(min_pt, max_pt);
        })
        .def("transformed_by", [](const AABB& self, const Pose3& pose) { return self.transformed_by(pose); })
        .def("transformed_by", [](const AABB& self, const GeneralPose3& pose) { return self.transformed_by(pose); })
        .def("__repr__", [](const AABB& aabb) {
            return "AABB(min_point=Vec3(" + std::to_string(aabb.min_point.x) + ", " +
                   std::to_string(aabb.min_point.y) + ", " + std::to_string(aabb.min_point.z) +
                   "), max_point=Vec3(" + std::to_string(aabb.max_point.x) + ", " +
                   std::to_string(aabb.max_point.y) + ", " + std::to_string(aabb.max_point.z) + "))";
        });
}

} // namespace termin
