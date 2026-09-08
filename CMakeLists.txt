cmake_minimum_required(VERSION 3.15)
set(PROJECT_NAME "NEE")
project(${PROJECT_NAME} LANGUAGES C CXX)

message("Building ${PROJECT_NAME}")
option(BUILD_TESTS "Build tests" ON)

find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(glm REQUIRED)
find_package(spdlog REQUIRED)
find_package(gsl-lite REQUIRED)
find_package(GTest REQUIRED)
find_package(stb REQUIRED)

include_directories(include)
file(GLOB_RECURSE SOURCES_LIST "src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES_LIST})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)

set_target_properties(
  ${PROJECT_NAME} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES
                             CXX_EXTENSIONS NO)

target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE
          glfw
    GLEW::GLEW
    nlohmann_json::nlohmann_json
    glm::glm
    spdlog::spdlog
    gsl::gsl-lite
    stb::stb)

enable_testing()
if(BUILD_TESTS)
  add_subdirectory(tests)
endif()

file(COPY ${CMAKE_SOURCE_DIR}/fonts
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/models
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/screenshots
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/shaders
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/textures
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_custom_command(
  TARGET ${PROJECT_NAME}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/shaders
          ${CMAKE_CURRENT_BINARY_DIR})
