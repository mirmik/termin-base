#!/usr/bin/env python3

from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
from setuptools.command.build import build as _build
from pathlib import Path
import shutil
import subprocess
import sys
import os


def _split_prefix_path(raw):
    if not raw:
        return []
    normalized = raw.replace(";", os.pathsep)
    return [p for p in normalized.split(os.pathsep) if p]


class CMakeBuild(_build):
    def run(self):
        self.run_command("build_ext")
        _build.run(self)


class CMakeBuildExt(build_ext):
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

        prefix_paths = _split_prefix_path(os.environ.get("CMAKE_PREFIX_PATH"))
        if prefix_paths:
            cmake_args.append(f"-DCMAKE_PREFIX_PATH={';'.join(prefix_paths)}")

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

        self._cmake_modules = modules
        self._cmake_ready = True

    def build_extension(self, ext):
        self._ensure_cmake_build()

        module_name = ext.name.rsplit(".", 1)[-1]
        built_module = self._cmake_modules.get(module_name)
        if not built_module:
            raise RuntimeError(f"Unknown module requested by setuptools: {module_name}")

        ext_path = Path(self.get_ext_fullpath(ext.name))
        ext_path.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(built_module, ext_path)

        # Also copy to source tree so build_py picks them up
        source_dir = Path(directory)
        pkg_dir = source_dir / "python" / "tcbase"
        pkg_dir.mkdir(parents=True, exist_ok=True)
        shutil.copy2(built_module, pkg_dir / built_module.name)


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
