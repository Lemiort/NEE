# AGENTS

This file contains short, actionable instructions for AI coding agents working on the NEE project.

## Build

```bash
# Resolve dependencies with Conan (build missing binaries)
conan install . --output-folder=build --build=missing -s build_type=Release -s compiler.cppstd=23

# Configure once with the Conan-generated preset (Ninja, Release)
cmake --preset conan-release -DBUILD_TESTING=ON

# Build (re-runs CMake by itself when CMakeLists change)
cmake --build build/build/Release
```

Conan's `cmake_layout` puts the CMake binary directory at `build/build/Release`;
that is the build tree to configure, build and test in. `build/build` itself is
not a build tree, so don't run `cmake --build build/build`.

The executable will be at `build/build/Release/NEE.exe`; test executables are in
`build/build/Release/tests/`.

## Test

The project uses **GoogleTest** for unit testing.

```bash
# Run all tests via CTest
ctest --test-dir build/build/Release --output-on-failure
```

## Common Pitfalls

- **Missing Conan profile**: Ensure a default profile exists (`~/.conan2/profiles/default`).
- **CMake cache**: Delete `build/` folder if you encounter stale configuration.
- **Duplicate preset `conan-release`**: `CMakeUserPresets.json` (git-ignored, written by `conan install`) includes the presets of every Conan output folder, and they all use that name. Remove the extra `include` entry (e.g. `build/vulkan/...`) or skip presets and run `cmake --build build/build/Release` on an already configured tree.

## Useful Tools

### Run and Build

- **Quick build**: Use the _Build_CMakeTools_ target to compile the current configuration.
- **Run executable**: After a successful build, execute `C:\Users\lemio.BEHEMOTH\Projects\NEE\build\build\Release\NEE.exe`.

### Linting & Static Analysis

- The project uses _clang‑tidy_; run:

```bash
clang-tidy -p=build/build/Release src/*.cpp
```

- Configure `-Werror` to treat warnings as errors during CI.

## Best Practices for Modern C++

This section summarizes key conventions used throughout the codebase:

| Topic                  | Guideline                                                                                                 |
| ---------------------- | --------------------------------------------------------------------------------------------------------- |
| **Language Standard**  | Use **C++23** (`-std=c++23`) with `CMAKE_CXX_STANDARD=23` and `CMAKE_CXX_EXTENSIONS OFF`.                 |
| **Memory Safety**      | Prefer smart pointers (`std::unique_ptr`, `std::shared_ptr`) and containers over raw pointers.            |
| **Move Semantics**     | Implement move constructors/assignment where appropriate; use `std::move` to avoid copies.                |
| **Exception Handling** | Reserve exceptions for truly exceptional cases; return `std::optional` or error codes otherwise.          |
| **Testing**            | The project uses GoogleTest for unit testing; tests reside in the `tests/` directory.                     |
| **Build tool**         | Use CMake targets: `add_executable`, `target_link_libraries`, and interface libraries for common headers. |
| **Static Analysis**    | Run clang‑tidy and cppcheck as part of CI; include `.clang-tidy`.                                         |

These guidelines aid agents in understanding the project’s coding style and help maintain consistency.

## Project Map

The following Mermaid diagram provides a high‑level view of the main components, useful for quick navigation by LLMs.

```mermaid
graph TD
    app[Demo app: app/ -> NEE.exe]
    src[Engine sources: src/ -> nee library]
    inc[Engine headers: include/]
    cfg[Configuration]
    conan[Conanfile]
    cmake[CMakeLists]
    build[Build Folder]
    test[Tests]
    shaders[Shaders]
    models[Models]
    textures[Textures]

    app --> src
    test --> src
    src --> inc
    src --> shaders
    src --> models
    src --> textures
    cmake --> build
    conan --> build
```
