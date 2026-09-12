cmake_minimum_required (VERSION 3.15 FATAL_ERROR)
set(PROJECT_NAME "NEE")
project (${PROJECT_NAME})


message("Building ${PROJECT_NAME}")


find_package(glfw3 REQUIRED)
find_package(glad REQUIRED)
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
file(GLOB_RECURSE HEADERS_LIST "include/*.hpp")

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

# --- Clang-format target ---------------------------------------------
add_custom_target(clang_format
    COMMENT "Formatting code with clang-format"
)
foreach(src_file IN LISTS SOURCES_LIST)
    add_custom_command(
        TARGET clang_format PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Formatting ${src_file}..."
        COMMAND clang-format -i "${src_file}"
    )
endforeach()


# --- Clang‑tidy target -----------------------------------------------
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_custom_target(clang_tidy
    COMMAND ${CMAKE_COMMAND} -E echo "Running clang-tidy..."
    # Run clang‑tidy with parallelism (use all cores)
    # The compile_commands.json is generated in the Release sub‑directory
    # by Ninja. Point clang‑tidy to that directory.
    COMMAND clang-tidy -p ${CMAKE_CURRENT_BINARY_DIR}/Release
        --header-filter=.
        ${SOURCES_LIST} ${HEADERS_LIST}
)

# link soil first to dismiss soil linking errors
# target_link_libraries(${PROJECT_NAME} ${SOIL_LIBRARIES})

target_link_libraries(${PROJECT_NAME} 
PRIVATE 
    glad::glad
    glfw
    stb::stb
    nlohmann_json::nlohmann_json
)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/config DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/fonts DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/models DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/screenshots DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/textures DESTINATION ${CMAKE_CURRENT_BINARY_DIR})