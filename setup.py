#!/usr/bin/env python3

from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
from setuptools.command.build import build as _build
from pathlib import Path
import shutil
import subprocess
import sys
import os


def _get_sdk_prefix():
    """On Windows, return path to shared termin-sdk directory."""
    if sys.platform == "win32":
        base = os.environ.get("LOCALAPPDATA", os.path.expanduser("~/AppData/Local"))
        return Path(base) / "termin-sdk"
    return None


def _copytree(src, dst):
    """Copy directory tree. On Windows, dereferences symlinks."""
    if dst.exists():
        shutil.rmtree(dst)
    follow = sys.platform == "win32"
    shutil.copytree(src, dst, symlinks=not follow)


class CMakeBuild(_build):
    """Run build_ext before build_py so that DLLs and artifacts copied to
    source tree during compilation are picked up by package_data."""

    def run(self):
        self.run_command("build_ext")
        # Now build_py will find DLLs/libs already in source tree
        _build.run(self)


class CMakeBuildExt(build_ext):
    """
    Build nanobind extension via CMake.
    CMake builds termin_base shared library + _tcbase_native Python module.
    Also installs C++ artifacts (headers, lib, cmake configs) into the package.
    """

    def _find_built_module(self, build_temp, cfg, module_name):
        patterns = [f"{module_name}.*.so", f"{module_name}.*.pyd", f"{module_name}.pyd"]
        built_files = []
        for pat in patterns:
            built_files.extend((build_temp / "python").glob(pat))
            built_files.extend((build_temp / "python" / cfg).glob(pat))
        return built_files[0] if built_files else None

    def _ensure_cmake_build(self):
        if getattr(self, "_cmake_ready", False):
            return

        source_dir = Path(directory)
        build_temp = Path(self.build_temp)
        build_temp.mkdir(parents=True, exist_ok=True)

        staging_dir = (build_temp / "install").resolve()
        staging_dir.mkdir(parents=True, exist_ok=True)

        cfg = "Debug" if self.debug else "Release"
        cmake_args = [
            str(source_dir),
            f"-DCMAKE_BUILD_TYPE={cfg}",
            f"-DTERMIN_BUILD_PYTHON=ON",
            f"-DPython_EXECUTABLE={sys.executable}",
            f"-DCMAKE_INSTALL_PREFIX={staging_dir}",
            f"-DCMAKE_INSTALL_LIBDIR=lib",
        ]

        subprocess.check_call(["cmake", *cmake_args], cwd=build_temp)
        subprocess.check_call(
            ["cmake", "--build", ".", "--config", cfg, "--parallel"],
            cwd=build_temp,
        )
        subprocess.check_call(
            ["cmake", "--install", ".", "--config", cfg],
            cwd=build_temp,
        )

        modules = {}
        for name in ["_tcbase_native", "_geom_native"]:
            built = self._find_built_module(build_temp, cfg, name)
            if not built:
                raise RuntimeError(f"CMake build did not produce {name} module")
            modules[name] = built

        self._cmake_source_dir = source_dir
        self._cmake_build_temp = build_temp
        self._cmake_staging_dir = staging_dir
        self._cmake_cfg = cfg
        self._cmake_modules = modules
        self._cmake_ready = True

    def build_extension(self, ext):
        self._ensure_cmake_build()

        source_dir = self._cmake_source_dir
        staging_dir = self._cmake_staging_dir
        module_name = ext.name.rsplit(".", 1)[-1]
        built_module = self._cmake_modules.get(module_name)
        if not built_module:
            raise RuntimeError(f"Unknown module requested by setuptools: {module_name}")

        # Copy to where setuptools expects it (makes install/bdist_wheel work)
        ext_path = Path(self.get_ext_fullpath(ext.name))
        ext_path.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(built_module, ext_path)

        # Copy C++ artifacts (headers, lib, cmake configs) next to the module
        ext_pkg_dir = ext_path.parent
        _install_cpp_artifacts(staging_dir, ext_pkg_dir)

        # On Windows, copy DLLs next to the .pyd (no RPATH on Windows)
        if sys.platform == "win32":
            _copy_dlls(staging_dir / "lib", ext_pkg_dir)

        # Also copy to source tree so build_py picks them up via package_data
        pkg_dir = source_dir / "python" / "tcbase"
        pkg_dir.mkdir(parents=True, exist_ok=True)
        shutil.copy2(built_module, pkg_dir / built_module.name)
        _install_cpp_artifacts(staging_dir, pkg_dir)

        if sys.platform == "win32":
            _copy_dlls(staging_dir / "lib", pkg_dir)

        # On Windows, also install C++ artifacts into shared SDK directory
        sdk = _get_sdk_prefix()
        if sdk:
            sdk.mkdir(parents=True, exist_ok=True)
            subprocess.check_call(
                ["cmake", "--install", ".", "--config", self._cmake_cfg, "--prefix", str(sdk)],
                cwd=self._cmake_build_temp,
            )


def _install_cpp_artifacts(staging_dir, pkg_dir):
    """Copy include/ and lib/ from cmake install staging to package dir."""
    if (staging_dir / "include").exists():
        _copytree(staging_dir / "include", pkg_dir / "include")
    if (staging_dir / "lib").exists():
        _copytree(staging_dir / "lib", pkg_dir / "lib")


def _copy_dlls(lib_dir, dst_dir):
    """Copy DLL files to target directory (Windows: DLLs must be next to .pyd)."""
    for dll in lib_dir.glob("*.dll"):
        shutil.copy2(dll, dst_dir / dll.name)


directory = os.path.dirname(os.path.realpath(__file__))

setup(
    name="tcbase",
    version="0.1.0",
    license="MIT",
    description="Base types shared between termin libraries",
    author="mirmik",
    author_email="mirmikns@yandex.ru",
    python_requires=">=3.8",
    packages=["tcbase"],
    package_dir={"tcbase": "python/tcbase"},
    package_data={
        "tcbase": [
            "include/**/*.h",
            "include/**/*.hpp",
            # Linux
            "lib/*.so*",
            # Windows
            "*.dll",
            "lib/*.dll",
            "lib/*.lib",
            # CMake configs
            "lib/cmake/termin_base/*.cmake",
        ],
    },
    ext_modules=[
        Extension("tcbase._tcbase_native", sources=[]),
        Extension("tcbase._geom_native", sources=[]),
    ],
    cmdclass={"build": CMakeBuild, "build_ext": CMakeBuildExt},
    zip_safe=False,
)
