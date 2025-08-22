# SPDX-FileCopyrightText: Leica Geosystems AG
#
# SPDX-License-Identifier: MIT

import os

from conan.tools.cmake import CMake, cmake_layout
from conan import ConanFile
from conan.tools.build import can_run

class SaamTestPackage(ConanFile):
    name = "saam_test_package"

    settings = "os", "compiler", "build_type", "arch"

    generators = "CMakeDeps", "CMakeToolchain", "VirtualRunEnv", "VirtualBuildEnv"

    def requirements(self):
        self.requires(self.tested_reference_str)

    def build(self):
        cmake = CMake(self)
        cmake.verbose = True
        cmake.configure()
        cmake.build()

    def layout(self):
        cmake_layout(self)

    def test(self):
        if can_run(self):
            cmd = os.path.join(self.cpp.build.bindir, self.name)
            self.run(cmd, env="conanrun")
