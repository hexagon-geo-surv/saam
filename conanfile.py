# SPDX-FileCopyrightText: Leica Geosystems AG
#
# SPDX-License-Identifier: MIT

from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout
import re



class SaamPackage(ConanFile):
    name = "saam"
    package_type = "header-library"

    settings = "os", "compiler", "build_type", "arch"

    license = "MIT"

    no_copy_source = True

    exports_sources = "include/*", "CMakeLists.txt", "conanfile.py", "test/*", "test_package/*", "template/*"

    generators = "CMakeDeps", "VirtualRunEnv", "VirtualBuildEnv"

    revision_mode = "scm"

    options = {
        "bc_mode": ["unchecked", "counted", "tracked"],
    }
    default_options = {
        "bc_mode": "counted"
    }

    @property
    def three_number_version(self):
        semver_pattern = '^(0|[1-9]\d*)\.(0|[1-9]\d*)\.(0|[1-9]\d*)(?:-((?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*)(?:\.(?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*))*))?(?:\+([0-9a-zA-Z-]+(?:\.[0-9a-zA-Z-]+)*))?$'
        parsed_version = re.search(semver_pattern, self.version)
        if parsed_version:
            return ".".join(parsed_version.group(1, 2, 3))

    def requirements(self):
        pass

    def build_requirements(self):
        self.test_requires("gtest/[^1.11.0]@")

    def generate(self):
        toolchain = CMakeToolchain(self)
        toolchain.cache_variables["CMAKE_VERBOSE_MAKEFILE"] = True
        toolchain.cache_variables["CMAKE_EXPORT_COMPILE_COMMANDS"] = True
        toolchain.cache_variables["SAAM_VERSION"] = self.three_number_version
        toolchain.cache_variables["SAAM_BORROW_CHECKING_MODE_CMAKE"] = {"unchecked":"2", "counted":"0", "tracked":"1"}[str(self.options.bc_mode)]
        toolchain.generate()

    def build(self):
        cmake = CMake(self)
        if self.should_configure:
            cmake.configure()

        if self.should_build:
            cmake.build()

        if self.should_test:
            cmake.test()

    def package(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.install()

    def layout(self):
        cmake_layout(self)

    def package_id(self):
        self.info.header_only()
        # because of the generated header in the build folder
        self.cpp.build.includedirs = [f"generated_code/{self.name}/include"]
