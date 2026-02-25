find_package(Python 3.8 COMPONENTS Interpreter Development REQUIRED)

execute_process(
    COMMAND python3 -m nanobind --cmake_dir
    OUTPUT_VARIABLE nanobind_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
find_package(nanobind CONFIG REQUIRED)

nanobind_add_module(_tcbase_native
    bindings/tcbase_module.cpp
)

target_link_libraries(_tcbase_native PRIVATE termin_base)

set_target_properties(_tcbase_native PROPERTIES
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH "$ORIGIN/lib"
)
