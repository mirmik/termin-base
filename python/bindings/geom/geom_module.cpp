#include "common.hpp"

namespace termin {

NB_MODULE(_geom_native, m) {
    m.doc() = "Native C++ geometry module for termin";

    // Bind all geometry types in order of dependencies
    bind_vec3(m);
    bind_vec4(m);
    bind_quat(m);
    bind_mat44(m);
    bind_pose3(m);
    bind_general_pose3(m);
    bind_screw3(m);
    bind_transform(m);
    bind_aabb(m);
}

} // namespace termin
