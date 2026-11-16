"""Conan recipe for NEE: dependencies for building it, and the nee package."""

import os
from typing import ClassVar

from conan import ConanFile
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout
from conan.tools.files import copy


class NeeConan(ConanFile):
    """NEE as a Conan package.

    Building NEE itself: `conan install .` then CMake, as in the README.
    Using it from a game: `requires("nee/<version>")`. During engine work a
    game can point at a checkout of NEE with `conan editable add <path>`,
    so engine edits need no republishing.
    """

    name = "nee"
    version = "0.1.0"
    description = "C++23 OpenGL 4.6 graphics engine"
    url = "https://github.com/Lemiort/NEE"
    package_type = "static-library"
    settings = "os", "arch", "compiler", "build_type"
    exports_sources = (
        "CMakeLists.txt",
        "include/*",
        "src/*",
        "app/*",
        "tests/*",
        "config/*",
        "fonts/*",
        "models/*",
        "shaders/*",
        "textures/*",
    )
    # GLAD is generated for the OpenGL version the engine renders with.
    default_options: ClassVar[dict[str, str]] = {
        "glad/*:gl_version": "4.6",
        "glad/*:gl_profile": "core",
    }

    # Runtime assets every NEE program loads: shipped as package resources.
    _asset_dirs = ("shaders", "fonts", "models", "textures")

    def requirements(self):
        """Declare the engine's dependencies and which games can see."""
        # Types from these appear in NEE's headers, so games compile against
        # them too.
        self.requires("glm/1.0.3", transitive_headers=True)
        self.requires("glad/2.0.8", transitive_headers=True)
        self.requires("gsl-lite/1.1.0", transitive_headers=True)
        self.requires("imgui/1.92.8", transitive_headers=True)
        # A game opens its window with GLFW and logs with spdlog alongside
        # the engine, so it gets them from NEE instead of requiring its own.
        self.requires("glfw/3.4", transitive_headers=True)
        self.requires("spdlog/1.17.0", transitive_headers=True)
        # Engine internals: linked in, but not visible to games.
        self.requires("stb/cci.20230920")
        self.requires("assimp/6.0.5")
        self.requires("glaze/8.4.0")

    def build_requirements(self):
        """Declare what only NEE's own tests need."""
        self.test_requires("gtest/1.14.0")

    def layout(self):
        """Lay out builds with CMake, and describe a checkout for editables."""
        cmake_layout(self)
        # In a checkout (editable mode) headers and assets come from the
        # source tree and libraries from the build tree.
        self.cpp.source.includedirs = ["include"]
        self.cpp.source.resdirs = ["."]
        self.cpp.build.libdirs = ["."]
        self.cpp.package.resdirs = ["res"]

    def generate(self):
        """Write the CMake toolchain and find_package files."""
        CMakeDeps(self).generate()
        CMakeToolchain(self).generate()

    def build(self):
        """Build the engine library."""
        cmake = CMake(self)
        # As a package only the library is needed; a checkout configured by
        # hand still builds the demo and tests.
        cmake.configure(variables={"NEE_BUILD_DEMO": "OFF", "NEE_BUILD_TESTS": "OFF"})
        cmake.build(target="nee")

    def package(self):
        """Copy headers, libraries and assets into the package."""
        copy(
            self,
            "*.hpp",
            os.path.join(self.source_folder, "include"),
            os.path.join(self.package_folder, "include"),
        )
        for pattern in ("*.a", "*.lib"):
            copy(
                self,
                pattern,
                self.build_folder,
                os.path.join(self.package_folder, "lib"),
                keep_path=False,
            )
        for directory in self._asset_dirs:
            copy(
                self,
                "*",
                os.path.join(self.source_folder, directory),
                os.path.join(self.package_folder, "res", directory),
            )

    def package_info(self):
        """Describe the nee::nee target for consumers."""
        self.cpp_info.set_property("cmake_file_name", "nee")
        self.cpp_info.set_property("cmake_target_name", "nee::nee")
        # The engine links its ImGui backends, so it comes first.
        self.cpp_info.libs = ["nee", "nee_imgui_backends"]
        self.cpp_info.defines = ["GLM_FORCE_CTOR_INIT", "GLFW_INCLUDE_NONE"]
        # Conan turns `settings` into an object with these fields.
        if self.settings.os in ("Linux", "FreeBSD"):  # pylint: disable=no-member
            self.cpp_info.system_libs = ["pthread", "dl"]
