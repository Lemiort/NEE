#ifndef INCLUDE_UTIL_H_
#define INCLUDE_UTIL_H_

#include <stdio.h>
#include <stdlib.h>

#include <cstring>
#include <sstream>
#include <string>

#define ZERO_MEM(a) memset(a, 0, sizeof(a))

#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a) / sizeof(a[0]))

#define INVALID_OGL_VALUE 0xFFFFFFFF

#define SAFE_DELETE(p) \
    if (p) {           \
        delete p;      \
        p = NULL;      \
    }

#define GLCheckError() (glGetError() == GL_NO_ERROR)

std::string ConvertToString(float number);

std::string ConvertToString(int number);

std::string ConvertToString(long number);

#endif  // INCLUDE_UTIL_H_
