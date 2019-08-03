#ifndef TGA_LOADER_H_INCLUDED
#define TGA_LOADER_H_INCLUDED
//#include "main.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cassert>
#include "Logger.h"

#define ASSERT(x) assert(x)
/*typedef __int8           int8_t;
typedef unsigned __int8  uint8_t;
typedef __int16          int16_t;
typedef unsigned __int16 uint16_t;
typedef __int32          int32_t;
typedef unsigned __int32 uint32_t;*/
//формат заголовков TGA
struct TGAHeader {
    uint8_t idlength;
    uint8_t colormap;
    uint8_t datatype;
    uint8_t colormapinfo[5];
    uint16_t xorigin;
    uint16_t yorigin;
    uint16_t width;
    uint16_t height;
    uint8_t bitperpel;
    uint8_t description;
};

bool LoadFile(const char *fileName, bool binary, uint8_t **buffer,
              uint32_t *size);
GLuint TextureCreateFromTGA(const char *fileName);

#endif  // TGA_LOADER_H_INCLUDED
