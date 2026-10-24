find_package(GTest REQUIRED)

add_executable(math_migration_tests
    MathMigrationTests.cpp
    ../src/Camera.cpp
    ../src/FontMetrics.cpp
    ../src/RenderQueue.cpp
    ../src/TextModel.cpp
    ../src/Transformable.cpp
)
target_include_directories(math_migration_tests PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_compile_definitions(math_migration_tests PRIVATE GLM_FORCE_CTOR_INIT)
target_link_libraries(math_migration_tests PRIVATE glm::glm glfw GTest::gtest_main)
set_target_properties(math_migration_tests PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)
include(GoogleTest)
gtest_discover_tests(math_migration_tests)

add_executable(material_texture_ownership_tests
    MaterialTextureOwnershipTests.cpp
    ../src/Material.cpp
    ../src/Texture.cpp
    ../src/util.cpp
)
target_include_directories(material_texture_ownership_tests PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(material_texture_ownership_tests PRIVATE
    glad::glad gsl-lite::gsl-lite stb::stb spdlog::spdlog GTest::gtest_main
)
set_target_properties(material_texture_ownership_tests PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)
gtest_discover_tests(material_texture_ownership_tests)
