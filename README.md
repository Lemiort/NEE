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

Install conan:
`conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan`
Setup conan:

```console
mkdir build && cd build
conan install .. --build=missing
```
