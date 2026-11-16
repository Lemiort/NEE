cmake_minimum_required (VERSION 3.15 FATAL_ERROR)
set(PROJECT_NAME "NEE")
project (${PROJECT_NAME})

# When NEE is consumed with add_subdirectory (for example as a git submodule of
# a game), only the engine library is built by default.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(NEE_IS_TOP_LEVEL ON)
else()
    set(NEE_IS_TOP_LEVEL OFF)
endif()
option(NEE_BUILD_DEMO "Build the NEE demo application" ${NEE_IS_TOP_LEVEL})
option(NEE_BUILD_TESTS "Build the NEE unit tests" ${NEE_IS_TOP_LEVEL})
# Directory holding the engine's runtime assets (shaders, models, fonts, ...).
set(NEE_ASSET_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "NEE asset root")

# GLAD owns all OpenGL declarations, including on platforms where GLFW would
# otherwise include the system's legacy OpenGL header.
add_compile_definitions(GLFW_INCLUDE_NONE)


message("Building ${PROJECT_NAME}")


find_package(glfw3 REQUIRED)
find_package(glad REQUIRED)
find_package(glm REQUIRED)
find_package(stb REQUIRED)
find_package(spdlog REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(gsl-lite CONFIG REQUIRED)
find_package(glaze CONFIG REQUIRED)
find_package(Threads REQUIRED)
find_package(imgui CONFIG REQUIRED)

# Conan ships Dear ImGui's platform and renderer backends as sources; build
# the ones the engine uses.
find_path(NEE_IMGUI_BACKEND_DIR imgui_impl_glfw.cpp
    PATHS
        "${imgui_PACKAGE_FOLDER_RELEASE}/res/bindings"
        "${imgui_PACKAGE_FOLDER_RELWITHDEBINFO}/res/bindings"
        "${imgui_PACKAGE_FOLDER_DEBUG}/res/bindings"
    NO_DEFAULT_PATH
)
if(NOT NEE_IMGUI_BACKEND_DIR)
    message(FATAL_ERROR "Dear ImGui backend sources not found in the imgui package")
endif()
add_library(nee_imgui_backends STATIC
    "${NEE_IMGUI_BACKEND_DIR}/imgui_impl_glfw.cpp"
    "${NEE_IMGUI_BACKEND_DIR}/imgui_impl_opengl3.cpp"
)
add_library(nee::imgui_backends ALIAS nee_imgui_backends)
target_include_directories(nee_imgui_backends PUBLIC "${NEE_IMGUI_BACKEND_DIR}")
target_link_libraries(nee_imgui_backends PUBLIC imgui::imgui glfw)
set_target_properties(nee_imgui_backends PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)


include(CTest)

file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE ENGINE_HEADERS CONFIGURE_DEPENDS "include/*.hpp")
file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS "app/*.cpp")
file(GLOB_RECURSE APP_HEADERS CONFIGURE_DEPENDS "app/*.hpp")
set(SOURCES_LIST ${ENGINE_SOURCES} ${APP_SOURCES})
set(HEADERS_LIST ${ENGINE_HEADERS} ${APP_HEADERS})

# --- Engine library ---------------------------------------------------
# Applications and tests link this target instead of compiling engine sources
# themselves. Dependencies that appear in public headers are PUBLIC; the rest
# stay PRIVATE and still reach the final link through the static library.
add_library(nee STATIC ${ENGINE_SOURCES} ${ENGINE_HEADERS})
add_library(nee::nee ALIAS nee)
target_include_directories(nee PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_compile_definitions(nee PUBLIC GLM_FORCE_CTOR_INIT GLFW_INCLUDE_NONE)
target_compile_features(nee PUBLIC cxx_std_23)
set_target_properties(nee PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)
target_link_libraries(nee
PUBLIC
    glad::glad
    glm::glm
    gsl-lite::gsl-lite
    Threads::Threads
    # Applications draw their UI with ImGui calls directly.
    imgui::imgui
PRIVATE
    nee_imgui_backends
    glfw
    stb::stb
    spdlog::spdlog
    assimp::assimp
    glaze::glaze
)

if(NEE_BUILD_TESTS AND BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(NOT NEE_BUILD_DEMO)
    return()
endif()

# --- Demo application -------------------------------------------------
add_executable(${PROJECT_NAME} ${APP_SOURCES} ${APP_HEADERS})
target_include_directories(${PROJECT_NAME} PRIVATE app)

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
    nee::nee
    glfw
    spdlog::spdlog
)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/config DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/fonts DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/models DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/screenshots DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/textures DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
