#include "TgaLoader.hpp"

#include <memory>
#include <vector>

bool LoadFile(const char* fileName, bool binary, uint8_t** buffer,
              uint32_t* size) {
    ASSERT(fileName);
    ASSERT(buffer);
    ASSERT(size);

    FILE* input;
    uint32_t fileSize, readed;

    const char mode[] = {'r', binary ? 'b' : 't', '\0'};
    if ((input = fopen(fileName, mode)) == NULL) {
        LOG_ERROR("Opening file '%s'\n", fileName);
        return false;
    }

    fseek(input, 0, SEEK_END);
    fileSize = (uint32_t)ftell(input);
    rewind(input);

    if (fileSize == 0) {
        LOG_ERROR("Empty file '%s'\n", fileName);
        fclose(input);
        return false;
    }

    std::vector<uint8_t> tempBuf(fileSize);
    ASSERT(tempBuf.get());

    readed = fread(tempBuf.data(), 1, fileSize, input);

    fclose(input);

    if (readed != fileSize) {
        LOG_ERROR("Reading file '%s'\n", fileName);
        return false;
    }

    // Allocate output buffer and copy data
    *buffer = new uint8_t[fileSize];
    std::memcpy(*buffer, tempBuf.data(), fileSize);

    *size = fileSize;
    return true;
}

// Function to load image from TGA file and create texture
GLuint TextureCreateFromTGA(const char* fileName) {
    ASSERT(fileName);

    TGAHeader* header;
    uint8_t* buffer;
    uint32_t size;
    GLint format, internalFormat;
    GLuint texture;

    // Try to load image from file
    if (!LoadFile(fileName, true, &buffer, &size)) return 0;

    // If file size is less than TGA header size
    if (size <= sizeof(TGAHeader)) {
        LOG_ERROR("Too small file '%s'\n", fileName);
        delete[] buffer;
        return 0;
    }

    header = (TGAHeader*)buffer;

    // Check TGA file format - uncompressed RGB or RGBA image
    if (header->datatype != 2 ||
        (header->bitperpel != 24 && header->bitperpel != 32)) {
        LOG_ERROR("Wrong TGA format '%s'\n", fileName);
        delete[] buffer;
        return 0;
    }

    // Get texture format
    format = (header->bitperpel == 24 ? GL_BGR : GL_BGRA);
    internalFormat = (format == GL_BGR ? GL_RGB8 : GL_RGBA8);

    // Request free texture index from OpenGL
    glGenTextures(1, &texture);

    // Make texture active
    glBindTexture(GL_TEXTURE_2D, texture);

    // Set texture filtering parameters - linear filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Set the "wrapping" parameters for the texture - no wrapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Load color data into the current active texture
    glTexImage2D(
        GL_TEXTURE_2D, 0, internalFormat, header->width, header->height, 0,
        format, GL_UNSIGNED_BYTE,
        (const GLvoid*)(buffer + sizeof(TGAHeader) + header->idlength));

    // Color data is no longer needed in memory after loading into the texture
    delete[] buffer;

    // Check for errors
    // OPENGL_CHECK_FOR_ERRORS();

    return texture;
}