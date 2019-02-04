cmake_minimum_required (VERSION 3.2.1 FATAL_ERROR)
set(PROJECT_NAME "NEE")
project (${PROJECT_NAME})

include(build/conanbuildinfo.cmake)
conan_basic_setup()

message("Building ${PROJECT_NAME}")


# enable_testing()

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

include_directories(src)
file(GLOB_RECURSE SOURCES_LIST "src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES_LIST})

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)


message("Trying to find glfw")

find_package(GLFW)

if(GLFW_FOUND)
    message("GLFW was found")
else()
    message(FATAL_ERROR "GLFW not found")
endif()

find_package("GLEW")
if(GLEW_FOUND)
    message("GLEW was found")
else()
    message(FATAL_ERROR "GLEW not found")
endif()

find_package(nlohmann_json 3.2.0 REQUIRED)

if(nlohmann_json_FOUND)
    message("nlohmann_json was found")
else()
    message(FATAL_ERROR "nlohmann_json not found")
endif()

find_package(GLM)

if(GLM_FOUND)
    message("GLM was found")
else()
    message(FATAL_ERROR "GLM not found")
endif()

find_package(SOIL)
if(SOIL_FOUND)
    message("SOIL was found")
    # include_directories(${SOIL_INCLUDE_DIRS})
else()
    message(FATAL_ERROR "SOIL not found")
endif()

find_package(Vulkan)
if(Vulkan_FOUND)
    message("Vulkan was found")
    # include_directories(${Vulkan_INCLUDE_DIRS})
else()
    message(FATAL_ERROR "Vulkan not found")
endif()


# link soil first to dismiss soil linking errors
target_link_libraries(${PROJECT_NAME} ${SOIL_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${CONAN_LIBS})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/config DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/fonts DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/models DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})