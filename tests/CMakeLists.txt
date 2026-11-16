find_package(GTest REQUIRED)
include(GoogleTest)

function(nee_add_test target)
    add_executable(${target} ${ARGN})
    target_link_libraries(${target} PRIVATE nee::nee GTest::gtest_main)
    set_target_properties(${target} PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
    )
endfunction()

nee_add_test(math_migration_tests MathMigrationTests.cpp)
target_link_libraries(math_migration_tests PRIVATE glfw)
gtest_discover_tests(math_migration_tests)

nee_add_test(fixed_step_clock_tests FixedStepClockTests.cpp)
gtest_discover_tests(fixed_step_clock_tests)

# Exercises a real worker thread and wall-clock pacing; waits are bounded.
nee_add_test(simulation_thread_tests SimulationThreadTests.cpp)
gtest_discover_tests(simulation_thread_tests PROPERTIES TIMEOUT 60)

# Dear ImGui scope guards on the headless null backend.
nee_add_test(imgui_scope_tests ImGuiScopeTests.cpp)
target_link_libraries(imgui_scope_tests PRIVATE nee::imgui_backends)
gtest_discover_tests(imgui_scope_tests)

nee_add_test(material_texture_ownership_tests
    MaterialTextureOwnershipTests.cpp
    GpuResourceOwnershipTests.cpp
)
gtest_discover_tests(material_texture_ownership_tests
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)

nee_add_test(opengl_render_tests OpenGLRenderTests.cpp)
target_compile_definitions(opengl_render_tests PRIVATE
    NEE_TEST_ASSET_ROOT="${PROJECT_SOURCE_DIR}"
    NEE_TEST_ARTIFACT_ROOT="${CMAKE_CURRENT_BINARY_DIR}/render-artifacts"
)
target_link_libraries(opengl_render_tests PRIVATE glfw stb::stb)
# Discovery only lists tests. GLFW is initialized inside each test, under Xvfb
# in CI. Run from the application build root, where runtime assets are copied.
gtest_discover_tests(opengl_render_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    PROPERTIES LABELS opengl TIMEOUT 60
)
