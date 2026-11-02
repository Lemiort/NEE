cmake_minimum_required (VERSION 3.15 FATAL_ERROR)
set(PROJECT_NAME "NEE")
project (${PROJECT_NAME})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


message("Building ${PROJECT_NAME}")


find_package(glfw3 REQUIRED)
find_package(glad REQUIRED)
find_package(glm REQUIRED)
find_package(stb REQUIRED)
find_package(spdlog REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(gsl-lite CONFIG REQUIRED)
find_package(glaze CONFIG REQUIRED)
find_package(volk CONFIG REQUIRED)

# Keep Vulkan independent of the OpenGL renderer while the backend is built out.
add_library(nee_vulkan STATIC src/VulkanBackend.cpp)
target_include_directories(nee_vulkan PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(nee_vulkan PUBLIC volk::volk PRIVATE glfw spdlog::spdlog)
target_compile_definitions(nee_vulkan PUBLIC VK_NO_PROTOTYPES PRIVATE GLFW_INCLUDE_NONE)
set_target_properties(nee_vulkan PROPERTIES
    CXX_STANDARD 23 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO
)

add_executable(NEEVulkanSmoke tools/VulkanSmoke.cpp)
target_link_libraries(NEEVulkanSmoke PRIVATE nee_vulkan glfw spdlog::spdlog)
target_compile_definitions(NEEVulkanSmoke PRIVATE GLFW_INCLUDE_NONE)
set_target_properties(NEEVulkanSmoke PROPERTIES
    CXX_STANDARD 23 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO
)


include(CTest)
option(NEE_ENABLE_VULKAN_SMOKE_TEST "Run the Vulkan smoke test (requires a display and Vulkan driver)" OFF)

if(BUILD_TESTING AND NEE_ENABLE_VULKAN_SMOKE_TEST)
    add_test(NAME vulkan_backend_smoke COMMAND NEEVulkanSmoke)
    set_tests_properties(vulkan_backend_smoke PROPERTIES LABELS vulkan TIMEOUT 30)
endif()

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

include_directories(include)
# include_directories(src)
file(GLOB_RECURSE SOURCES_LIST CONFIGURE_DEPENDS "src/*.cpp")
list(REMOVE_ITEM SOURCES_LIST "${CMAKE_CURRENT_SOURCE_DIR}/src/VulkanBackend.cpp")
file(GLOB_RECURSE HEADERS_LIST CONFIGURE_DEPENDS "include/*.hpp")

add_executable(${PROJECT_NAME} ${SOURCES_LIST})
target_compile_definitions(${PROJECT_NAME} PRIVATE GLM_FORCE_CTOR_INIT)

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
    glm::glm
    stb::stb
    spdlog::spdlog
    assimp::assimp
    gsl-lite::gsl-lite
    glaze::glaze
)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/config DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/fonts DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/models DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/screenshots DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/textures DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
