#pragma once

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#if __cplusplus
extern "C" {
#endif

#define LOGGER_CONSOLE (1)
#define LOGGER_SINK    (0)

enum {
    LOG_DEBUG = 1,
    LOG_INFO  = 2,
    LOG_WARN  = 3,
    LOG_ERROR = 4,
    LOG_FATAL = 5,
};

typedef void (*elf_logger_t)(const int level, const char* log);


void elf_set_logger(elf_logger_t logger);

void elf_log_vwrite(const int level, const char* fmt, ...);

#define log_dbg(fmt...)     elf_log_vwrite(LOG_DEBUG,  ##fmt)
#define log_info(fmt...)    elf_log_vwrite(LOG_INFO,  ##fmt)
#define log_warn(fmt...)    elf_log_vwrite(LOG_WARN,   ##fmt)
#define log_error(fmt...)   elf_log_vwrite(LOG_ERROR, ##fmt)
#define log_fatal(fmt...)   elf_log_vwrite(LOG_FATAL,  ##fmt)

#if __cplusplus
};
#endif