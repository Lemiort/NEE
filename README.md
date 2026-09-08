New Exprience Engine

[![CMake and Conan 2 CI](https://github.com/Lemiort/NEE/actions/workflows/build.yaml/badge.svg)](https://github.com/Lemiort/NEE/actions/workflows/build.yaml)

# README #
This is realization of Openg3.3+ graphics engine. Adapted to conan2


![Screenshot 22.08.2015](screenshots/demo_screenshot_26-08-2015.jpg)


# Building

Use following command to configure cmake before building

```bash
conan install . --output-folder=build --build=missing -s compiler.cppstd=23
```
