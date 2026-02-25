cmake_minimum_required(VERSION 3.16)
project(termin_base VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# termin_base library (tc_log + trent + settings)
add_library(termin_base SHARED
    src/tc_log.c
    src/trent/trent.cpp
    src/trent/trent_path.cpp
    src/trent/string_ext.cpp
    src/trent/json.cpp
    src/trent/yaml.cpp
    src/settings.cpp
)

target_include_directories(termin_base PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/tcbase/trent>
    $<INSTALL_INTERFACE:include>
    $<INSTALL_INTERFACE:include/tcbase/trent>
)

target_compile_definitions(termin_base PRIVATE TCBASE_EXPORTS)

# Python bindings
option(TCBASE_BUILD_PYTHON "Build Python bindings" OFF)

if(TCBASE_BUILD_PYTHON)
    add_subdirectory(python)
endif()

# Install
include(GNUInstallDirs)

install(TARGETS termin_base
    EXPORT termin_baseTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY include/tcbase
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# CMake config for find_package
install(EXPORT termin_baseTargets
    FILE termin_baseTargets.cmake
    NAMESPACE tcbase::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/termin_base
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
    cmake/termin_baseConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/termin_baseConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/termin_base
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/termin_baseConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/termin_baseConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/termin_baseConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/termin_base
)
