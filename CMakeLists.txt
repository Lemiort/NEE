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

file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

message("Trying to find glfw")

find_package("GLFW")

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

target_link_libraries(${PROJECT_NAME} ${CONAN_LIBS})


# # Make sure the compiler can find include files for our Hello library
# # when other libraries or executables link to Hello
# target_include_directories (${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

