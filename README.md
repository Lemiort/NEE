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

Setup conan:
```bash
conan profile new default --detect
conan profile update settings.compiler.libcxx=libstdc++11 default
conan remote add bincrafters https://bincrafters.jfrog.io/artifactory/api/conan/public-conan
conan config set general.revisions_enabled=1
```

Install conan packages:

```console
mkdir build && cd build
conan install .. --build=missing
```
