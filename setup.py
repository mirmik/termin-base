#!/usr/bin/env python3

from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
from pathlib import Path
import shutil
import subprocess
import sys
import os


class CMakeBuildExt(build_ext):
    """
    Build nanobind extension via CMake.
    CMake builds termin_base (header-only) + _tcbase_native module.
    """

    def build_extension(self, ext):
        source_dir = Path(directory)
        build_temp = Path(self.build_temp)
        build_temp.mkdir(parents=True, exist_ok=True)

        cfg = "Debug" if self.debug else "Release"
        cmake_args = [
            str(source_dir),
            f"-DCMAKE_BUILD_TYPE={cfg}",
            f"-DTCBASE_BUILD_PYTHON=ON",
            f"-DPython_EXECUTABLE={sys.executable}",
        ]

        subprocess.check_call(["cmake", *cmake_args], cwd=build_temp)
        subprocess.check_call(
            ["cmake", "--build", ".", "--config", cfg, "--parallel"],
            cwd=build_temp,
        )

        # Find the built .so
        built_files = list((build_temp / "python").glob("_tcbase_native.*"))
        if not built_files:
            raise RuntimeError("CMake build did not produce _tcbase_native module")

        built_so = built_files[0]

        # Copy to where setuptools expects it (makes install/bdist_wheel work)
        ext_path = Path(self.get_ext_fullpath(ext.name))
        ext_path.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(built_so, ext_path)

        # Also copy to source tree for editable/development use
        pkg_dir = source_dir / "python" / "tcbase"
        pkg_dir.mkdir(parents=True, exist_ok=True)
        shutil.copy2(built_so, pkg_dir / built_so.name)


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
    ext_modules=[
        Extension("tcbase._tcbase_native", sources=[]),
    ],
    cmdclass={"build_ext": CMakeBuildExt},
    zip_safe=False,
)
