message("-- Conan libs: ${CONAN_LIBS}")

add_executable(
  UnitTests
  UnitTests.cpp
)
target_link_libraries(
    UnitTests
    ${CONAN_LIBS}
)
# include(GoogleTest)
# gtest_discover_tests(UnitTests)

add_test(NAME  GTests 
        COMMAND  UnitTests
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
