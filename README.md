# README #
This is realization of Openg3.3+ graphics engine. To compile project you should use Code::Blocks.
Project have 4 targets:
* Debug
* Release
* Debug_x64
* Release_x64
To compile Debug and Release you can use regular GCC or Mingw64 as 32-bit compiler cause at this moment regular MinGW doesn't supports C++11. To compile x64 targets, use GCC or Mingw64.

Also to compile you should point path to this libraries in C::B global variables:
* glew-32bit
* glew-64bit
* glfw-32bit
* glfw-64bit
You can use 32-bit or 64-bit libraries, or both on the discretion.

conan remote add lemiort https://api.bintray.com/conan/lemiort/conan-public
conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan