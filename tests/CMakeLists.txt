message("-- Conan libs: ${CONAN_LIBS}")

file(GLOB_RECURSE TESTS_SRC ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)
message("-- Test sources: ${TESTS_SRC}")

add_executable(
  UnitTests
  ${TESTS_SRC}
)

target_include_directories(
    UnitTests
  PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(
    UnitTests
    neegine
    CONAN_PKG::gtest
)
# include(GoogleTest)
# gtest_discover_tests(UnitTests)

add_test(NAME  GTests 
        COMMAND  UnitTests
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
