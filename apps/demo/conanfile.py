# SPDX-FileCopyrightText: Leica Geosystems AG
#
# SPDX-License-Identifier: MIT

from conan import ConanFile
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout
from conan.tools.env import VirtualBuildEnv, VirtualRunEnv
from conan.tools.scm import Version 
from conan.tools.files import copy, rmdir


class SaamDemoAppPackage(ConanFile):
    name = "saam-demo"
    package_type = "application"

    settings = "os", "compiler", "build_type", "arch"

    no_copy_source = True

    exports_sources = "src/*", "CMakeLists.txt", "conanfile.py"

    generators = "CMakeDeps", "VirtualRunEnv", "VirtualBuildEnv"

    def requirements(self):
        self.requires(f"saam/{self.version}@sgo/stable", transitive_headers=True)

    def generate(self):
        toolchain = CMakeToolchain(self)
        toolchain.cache_variables["CMAKE_VERBOSE_MAKEFILE"] = True
        toolchain.cache_variables["CMAKE_EXPORT_COMPILE_COMMANDS"] = True
        toolchain.generate()

    def build(self):
        cmake = CMake(self)
        if self.should_configure:
            cmake.configure()

        if self.should_build:
            cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.install()

    def layout(self):
        cmake_layout(self)
