#ifndef INCLUDE_LOGGER_H
#define INCLUDE_LOGGER_H

#include <cstdarg>
#include <cstdio>
#include <cstring>

#if defined(_MSC_VER)
#define LOG_DEBUG(str, ...)                                              \
    LoggerWrite("%s:%s:%d DEBUG " str, __FILE__, __FUNCTION__, __LINE__, \
                ##__VA_ARGS__)
#define LOG_ERROR(str, ...)                                              \
    LoggerWrite("%s:%s:%d ERROR " str, __FILE__, __FUNCTION__, __LINE__, \
                ##__VA_ARGS__)
#else
#define LOG_DEBUG(str, ...)                                          \
    LoggerWrite("%s:%s:%d DEBUG " str, __FILE__, __func__, __LINE__, \
                ##__VA_ARGS__)
#define LOG_ERROR(str, ...)                                          \
    LoggerWrite("%s:%s:%d ERROR " str, __FILE__, __func__, __LINE__, \
                ##__VA_ARGS__)
#endif

#define ASSERT(x) assert(x)

void LoggerCreate(const char *fileName);

void LoggerDestroy();

void LoggerWrite(const char *format, ...);

#endif  // INCLUDE_LOGGER_H
