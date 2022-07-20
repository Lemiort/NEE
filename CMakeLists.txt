cmake_minimum_required(VERSION 3.2.1 FATAL_ERROR)
set(PROJECT_NAME "NEE")
project(${PROJECT_NAME})

include(build/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

message("Building ${PROJECT_NAME}")
option(BUILD_TESTS "Build tests" ON)

enable_testing()

if(BUILD_TESTS)
  add_subdirectory(tests)
endif()

include_directories(include)
add_subdirectory(src)

add_executable(${PROJECT_NAME} 
     main.cpp
)

# link soil first to dismiss soil linking errors
# target_link_libraries(${PROJECT_NAME} ${SOIL_LIBRARIES})
target_link_libraries(${PROJECT_NAME} neegine)

# file(COPY ${CMAKE_SOURCE_DIR}/config
#      DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
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

# to update shader sources ervry build
add_custom_command(
  TARGET ${PROJECT_NAME}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/shaders
          ${CMAKE_CURRENT_BINARY_DIR})
