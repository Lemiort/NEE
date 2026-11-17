# Running tests

Configure with `BUILD_TESTING=ON`, build, then run CTest in the CMake binary
directory. For the GitHub workflow this is `build`; for the local Conan Release
preset it is `build/build/Release`.

```sh
ctest --test-dir build -C Release --output-on-failure --no-tests=error -LE opengl
LIBGL_ALWAYS_SOFTWARE=1 MESA_GL_VERSION_OVERRIDE=4.6 MESA_GLSL_VERSION_OVERRIDE=460 \
  xvfb-run -a -s "-screen 0 1280x720x24" \
  ctest --test-dir build -C Release --output-on-failure --no-tests=error -L opengl
```

The first command executes the CPU and mocked ownership tests. The second
executes real OpenGL tests under Xvfb using Mesa software rendering on Linux.
The Mesa overrides exercise the 4.6 context and GLSL path on llvmpipe releases
that advertise 4.5. They are only for software CI; desktop runs require an
actual OpenGL 4.6 driver and should omit the overrides.
Install `xvfb`, `xauth`, and Mesa along with the GLFW development dependencies.
On a desktop with a working OpenGL driver, run CTest directly without Xvfb.
Context creation failures fail the tests rather than silently skipping them.

## OpenGL coverage

- Repository texture, cubemap, and model loaders use explicit source-root paths.
- Production mesh, G-buffer, lighting, text, debug, skybox, billboard, and stencil
  shader programs compile/link using the production Shader class.
- The production G-buffer creates complete attachments and resizes without GL
  errors.
- The deferred pipeline, with no lights, shows a material's emissive color,
  multiplies an emissive texture by that color, and leaves surfaces without
  one black. The emissive texture binds to the last texture unit.
- Light above 1 survives in the half-float light target until tone mapping:
  halving the exposure of an emissive 2 gives full brightness. Reinhard and
  ACES map light of 1 to 0.5 and about 0.8.
- Bloom spills a bright cube's light onto nearby black pixels only when
  enabled, and not from a cube glowing below the threshold. Its levels
  halve down to two texels a side.
- Particles are born at random places and ages inside their emitter box,
  fall with their velocity, follow acceleration, are born again when their
  lifetime ends, and draw as camera-facing squares or streaks, also through
  the render queue in the deferred pipeline.
- A material with a world texture scale takes its texture coordinates from
  the world: moving a cube by half a tile swaps the colors on its face.
- DSA texture uploads, queries, random textures, and framebuffer resizing preserve
  existing bindings. Uploaded textures have immutable storage and mip levels.
- Text, debug lines, billboards, and skyboxes draw with their own vertex arrays
  after the previously bound VAO has been cleared.
- The production Mesh and MeshInstanceRenderer draw two differently placed,
  rotated, and scaled instances to an offscreen framebuffer. Images from
  individual and batched drawing must agree within one 8-bit channel value.
- Both production mesh vertex shaders must produce matching diagnostic images.
  Coverage assertions ensure both instances are visible, so empty frames cannot
  satisfy image comparison.

The image test uses a diagnostic fragment shader consuming normals, tangents,
and UVs. It verifies geometry/instancing consistency, not equality of the full
forward/deferred lighting implementations. Full lit-scene comparisons remain
follow-up work for issue #41, alongside the lighting fixes in #22, #24, and #26.
The asset test verifies loaders and repository assets, not the application's
hardcoded asset path casing (#18).

No GPU resource is constructed during GoogleTest discovery. Every OpenGL test
creates a hidden OpenGL 4.6 core context, verifies the version/profile, and
destroys resources before destroying that context. Synchronous debug output
fails tests on GL errors or undefined behavior. CPU and mocked GL tests run in
separate executables.

On image-comparison failure, PPM reference/result images are saved beneath the
test binary directory in `render-artifacts/`. GitHub Actions uploads these images
and CTest's `Testing/Temporary` logs when a job fails.
