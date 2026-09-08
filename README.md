# README

- [README](#readme)
  - [About](#about)
  - [Requrements](#requrements)
  - [Building](#building)

## About

This is realization of Openg3.3+ graphics engine.

## Requrements

-conan
-c++17 compiler
-cmake

## Building

Setup Conan 2:
```bash
conan profile detect --force
conan install . --output-folder=build --build=missing -s compiler.cppstd=17
```

Configure and build:

```console
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=build/build/generators/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
```
