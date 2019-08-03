#include "tga_loader.h"

bool LoadFile(const char *fileName, bool binary, uint8_t **buffer,
              uint32_t *size) {
    ASSERT(fileName);
    ASSERT(buffer);
    ASSERT(size);

    FILE *input;
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

    *buffer = new uint8_t[fileSize];
    ASSERT(*buffer);

    readed = fread(*buffer, 1, fileSize, input);

    fclose(input);

    if (readed != fileSize) {
        LOG_ERROR("Reading file '%s'\n", fileName);
        delete[] * buffer;
        return false;
    }

    *size = fileSize;

    return true;
}

// функция загрузки изображения из файла TGA и сздания текстуры
GLuint TextureCreateFromTGA(const char *fileName) {
    ASSERT(fileName);

    TGAHeader *header;
    uint8_t *buffer;
    uint32_t size;
    GLint format, internalFormat;
    GLuint texture;

    // попытаемся загрузить изображение из файла
    if (!LoadFile(fileName, true, &buffer, &size)) return 0;

    // если размер файла заведомо меньше заголовка TGA
    if (size <= sizeof(TGAHeader)) {
        LOG_ERROR("Too small file '%s'\n", fileName);
        delete[] buffer;
        return 0;
    }

    header = (TGAHeader *)buffer;

    // проверим формат TGA-файла - несжатое RGB или RGBA изображение
    if (header->datatype != 2 ||
        (header->bitperpel != 24 && header->bitperpel != 32)) {
        LOG_ERROR("Wrong TGA format '%s'\n", fileName);
        delete[] buffer;
        return 0;
    }

    // получим формат текстуры
    format = (header->bitperpel == 24 ? GL_BGR : GL_BGRA);
    internalFormat = (format == GL_BGR ? GL_RGB8 : GL_RGBA8);

    // запросим у OpenGL свободный индекс текстуры
    glGenTextures(1, &texture);

    // сделаем текстуру активной
    glBindTexture(GL_TEXTURE_2D, texture);

    // установим параметры фильтрации текстуры - линейная фильтрация
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // установим параметры "оборачивания" текстуры - отсутствие оборачивания
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // загрузим данные о цвете в текущую автивную текстуру
    glTexImage2D(
        GL_TEXTURE_2D, 0, internalFormat, header->width, header->height, 0,
        format, GL_UNSIGNED_BYTE,
        (const GLvoid *)(buffer + sizeof(TGAHeader) + header->idlength));

    // после загрузки в текстуру данные о цвете в памяти нам больше не нужны
    delete[] buffer;

    // проверим на наличие ошибок
    // OPENGL_CHECK_FOR_ERRORS();

    return texture;
}