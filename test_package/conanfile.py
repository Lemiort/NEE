"""Conan test_package: builds a small program against the nee package."""

import os

from conan import ConanFile
from conan.tools.build import can_run
from conan.tools.cmake import CMake, cmake_layout


class NeeTestPackage(ConanFile):
    """Builds a tiny program against the packaged engine, as a game does."""

    settings = "os", "arch", "compiler", "build_type"
    generators = "CMakeDeps", "CMakeToolchain"

    def requirements(self):
        """Require the package under test."""
        self.requires(self.tested_reference_str)

    def layout(self):
        """Lay out the build with CMake."""
        cmake_layout(self)

    def build(self):
        """Build the test program."""
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def test(self):
        """Run the test program when it can run on this machine."""
        if can_run(self):
            program = os.path.join(self.cpp.build.bindir, "nee_package_test")
            self.run(program, env="conanrun")
