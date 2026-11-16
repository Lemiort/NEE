cmake_minimum_required(VERSION 3.21)
project(NeePackageTest LANGUAGES CXX)

# A game needs only the engine: GLFW, spdlog, GLM and ImGui come with it.
find_package(nee REQUIRED CONFIG)

add_executable(nee_package_test main.cpp)
target_link_libraries(nee_package_test PRIVATE nee::nee)
target_compile_features(nee_package_test PRIVATE cxx_std_23)
