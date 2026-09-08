file(GLOB_RECURSE TESTS_SRC ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)
message("-- Test sources: ${TESTS_SRC}")

add_executable(UnitTests ${TESTS_SRC})

target_include_directories(UnitTests PRIVATE ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(
  UnitTests
  PRIVATE
    GTest::gtest
    GTest::gtest_main
    glfw
    GLEW::GLEW
    nlohmann_json::nlohmann_json
    glm::glm
    spdlog::spdlog
    gsl::gsl-lite)

add_test(NAME GTests COMMAND UnitTests WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
