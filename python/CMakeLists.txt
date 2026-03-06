find_package(Python 3.8 COMPONENTS Interpreter Development REQUIRED)
find_package(nanobind CONFIG REQUIRED)

nanobind_add_module(_tcbase_native
    bindings/tcbase_module.cpp
)

target_link_libraries(_tcbase_native PRIVATE termin_base)

nanobind_add_module(_geom_native
    bindings/geom/geom_module.cpp
    bindings/geom/vec3.cpp
    bindings/geom/vec4.cpp
    bindings/geom/quat.cpp
    bindings/geom/mat44.cpp
    bindings/geom/pose3.cpp
    bindings/geom/general_pose3.cpp
    bindings/geom/screw3.cpp
    bindings/geom/aabb.cpp
)

target_link_libraries(_geom_native PRIVATE termin_base)

if(NOT WIN32)
    set_target_properties(_tcbase_native PROPERTIES
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH "/opt/termin/lib"
    )
    set_target_properties(_geom_native PROPERTIES
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH "/opt/termin/lib"
    )
endif()
