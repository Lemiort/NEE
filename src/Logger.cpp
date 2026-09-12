#include "Logger.hpp"

#include <cstdarg>
#include <cstdio>
#include <cstring>

static const int LOGGER_FILENAME_MAX = 256;
static char g_LoggerFileName[LOGGER_FILENAME_MAX] = "lesson.log";

void LoggerCreate(const char* fileName) {
  FILE* output = nullptr;

  memset(g_LoggerFileName, 0, LOGGER_FILENAME_MAX);
  strncpy(g_LoggerFileName, fileName, LOGGER_FILENAME_MAX - 1);

  if ((output = fopen(g_LoggerFileName, "w")) != nullptr) {
    fclose(output);
  }
}

void LoggerDestroy() {
    //
}

void LoggerWrite(const char* format, ...) {
  va_list ap = nullptr;
  FILE* output = nullptr;

  if ((output = fopen(g_LoggerFileName, "a+")) == nullptr) {
    return;
  }

    va_start(ap, format);
    vfprintf(output, format, ap);
    va_end(ap);

    fclose(output);
}
