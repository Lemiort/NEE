cmake_minimum_required (VERSION 3.15 FATAL_ERROR)
set(PROJECT_NAME "NEE")
project (${PROJECT_NAME})


message("Building ${PROJECT_NAME}")


find_package(glfw3 REQUIRED)
find_package(glew REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(glm REQUIRED)
find_package(stb REQUIRED)


enable_testing()

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

include_directories(include)
# include_directories(src)
file(GLOB_RECURSE SOURCES_LIST "src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES_LIST})

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

# # Enable compiler warnings and treat them as errors for CI safety
# if(MSVC)
#     target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
# else()
#     target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
# endif()

# Export compile commands for clang-tidy and other tooling
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# link soil first to dismiss soil linking errors
# target_link_libraries(${PROJECT_NAME} ${SOIL_LIBRARIES})

target_link_libraries(${PROJECT_NAME} 
PRIVATE 
    GLEW::GLEW
    glfw
    stb::stb
)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/config DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/fonts DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/models DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/screenshots DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/textures DESTINATION ${CMAKE_CURRENT_BINARY_DIR})