New Exprience Engine

[![CMake and Conan 2 CI](https://github.com/Lemiort/NEE/actions/workflows/build.yaml/badge.svg)](https://github.com/Lemiort/NEE/actions/workflows/build.yaml)

# README #
NEE is a C++23 graphics engine using OpenGL 4.6 core and Conan 2.
Running it requires a desktop driver supporting OpenGL 4.6 and GLSL 4.60.
The system OpenGL implementation on macOS does not meet this requirement.

## OpenGL resource setup

The renderer uses [Direct State Access](https://wikis.khronos.org/opengl/Direct_State_Access)
to configure named textures, buffers, framebuffers, vertex arrays, and transform
feedback objects without temporary bindings. Static mesh data is uploaded
directly from interleaved model vertices into immutable buffer storage. Mesh
vertex layouts are cached and reused for individual and instanced draws;
text, debug lines, billboards, skyboxes, and fullscreen passes own their VAOs.

Textures use sized immutable storage with explicit mip levels. Resizing render
targets creates replacement storage. Material textures bind in batches with
`glBindTextures`, and material uniforms use `glProgramUniform*` to address their
program directly. Text and instance streams retain resizable storage through
`glNamedBufferData`.

Texture uploads, texture queries, and framebuffer setup leave existing bindings unchanged.
Rendering still binds its VAO, program, render target, and texture units.
Texture `Bind` methods accept `GL_TEXTURE0 + unit` as before; they now select
the unit directly without changing the active texture selector.

## Coordinates and units

The world is right handed: +X is right, +Y is up, and the default camera looks
along -Z. Positive rotations follow the right hand rule. Front faces use
counterclockwise winding, and OpenGL clip depth is -1 to +1. Camera FOV,
object Euler angles, and spotlight cutoff are specified in degrees at the engine
API; GLM calculations use radians internally. Treat one world unit as one meter
when authoring scene values. Imported model coordinates are used as stored.

## Runtime diagnostics

The application writes logs to `NEE.log` beside the executable and to standard
error. 

![Screenshot 22.08.2015](screenshots/demo_screenshot_26-08-2015.jpg)


# Building

Resolve dependencies and build Release:

```bash
conan install . --output-folder=build --build=missing -s build_type=Release -s compiler.cppstd=23
cmake --preset conan-release -DBUILD_TESTING=ON
cmake --build --preset conan-release
ctest --test-dir build/build/Release -C Release --output-on-failure
```

`conanfile.txt` explicitly generates GLAD for OpenGL 4.6 core. Re-run Conan
when updating an existing checkout so the loader matches the renderer. Use
`-pr:a mingw` with Conan when building with a configured LLVM-MinGW profile.
Run the executable from the CMake binary directory so it can find the copied
`shaders`, `textures`, `models`, `fonts`, and `config` directories.
