file(GLOB_RECURSE SOURCES_LIST "*.cpp")

add_library(neegine STATIC
    ${SOURCES_LIST}
)

set_target_properties(neegine 
    PROPERTIES 
        CXX_STANDARD 17 
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
)

target_link_libraries(neegine
    PUBLIC
        CONAN_PKG::glfw
        CONAN_PKG::glew
        CONAN_PKG::nlohmann_json
        CONAN_PKG::glm
        CONAN_PKG::soil2
        CONAN_PKG::spdlog
        CONAN_PKG::gsl-lite
)
