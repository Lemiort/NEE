#include "tga_loader.h"

bool LoadFile(const char *fileName, bool binary, uint8_t **buffer, uint32_t *size)
{
	ASSERT(fileName);
	ASSERT(buffer);
	ASSERT(size);

	FILE     *input;
	uint32_t fileSize, readed;

	const char mode[] = {'r', binary ? 'b' : 't', '\0'};
	if ((input = fopen(fileName, mode)) == NULL)
	{
		LOG_ERROR("Opening file '%s'\n", fileName);
		return false;
	}

	fseek(input, 0, SEEK_END);
	fileSize = (uint32_t)ftell(input);
	rewind(input);

	if (fileSize == 0)
	{
		LOG_ERROR("Empty file '%s'\n", fileName);
		fclose(input);
		return false;
	}

	*buffer = new uint8_t[fileSize];
	ASSERT(*buffer);

	readed = fread(*buffer, 1, fileSize, input);

	fclose(input);

	if (readed != fileSize)
	{
		LOG_ERROR("Reading file '%s'\n", fileName);
		delete[] *buffer;
		return false;
	}

	*size = fileSize;

	return true;
}


// ������� �������� ����������� �� ����� TGA � ������� ��������
GLuint TextureCreateFromTGA(const char *fileName)
{
  ASSERT(fileName);

  TGAHeader *header;
  uint8_t   *buffer;
  uint32_t  size;
  GLint     format, internalFormat;
  GLuint    texture;

  // ���������� ��������� ����������� �� �����
  if (!LoadFile(fileName, true, &buffer, &size))
    return 0;

  // ���� ������ ����� �������� ������ ��������� TGA
  if (size <= sizeof(TGAHeader))
  {
    LOG_ERROR("Too small file '%s'\n", fileName);
    delete[] buffer;
    return 0;
  }

  header = (TGAHeader*)buffer;

  // �������� ������ TGA-����� - �������� RGB ��� RGBA �����������
  if (header->datatype != 2 || (header->bitperpel != 24 && header->bitperpel != 32))
  {
    LOG_ERROR("Wrong TGA format '%s'\n", fileName);
    delete[] buffer;
    return 0;
  }

  // ������� ������ ��������
  format = (header->bitperpel == 24 ? GL_BGR : GL_BGRA);
  internalFormat = (format == GL_BGR ? GL_RGB8 : GL_RGBA8);

  // �������� � OpenGL ��������� ������ ��������
  glGenTextures(1, &texture);

  // ������� �������� ��������
  glBindTexture(GL_TEXTURE_2D, texture);

  // ��������� ��������� ���������� �������� - �������� ����������
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // ��������� ��������� "������������" �������� - ���������� ������������
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  // �������� ������ � ����� � ������� �������� ��������
  glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, header->width, header->height, 0, format,
    GL_UNSIGNED_BYTE, (const GLvoid*)(buffer + sizeof(TGAHeader) + header->idlength));

  // ����� �������� � �������� ������ � ����� � ������ ��� ������ �� �����
  delete[] buffer;

  // �������� �� ������� ������
  //OPENGL_CHECK_FOR_ERRORS();

  return texture;
}