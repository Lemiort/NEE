cmake_minimum_required (VERSION 3.2.1 FATAL_ERROR)
set(PROJECT_NAME "NEE")
project (${PROJECT_NAME})

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

# # Make sure the compiler can find include files for our Hello library
# # when other libraries or executables link to Hello
# target_include_directories (${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

