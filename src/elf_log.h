
#pragma once

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define log_info(...)   do{ fprintf(stdout, __VA_ARGS__); } while(0)
#define log_error(...)  do{ fprintf(stdout, __VA_ARGS__); } while(0)
#define log_warn(...)   do{ fprintf(stdout, __VA_ARGS__); } while(0)
#define log_fatal(...)  do{ fprintf(stdout, __VA_ARGS__); } while(0)
#define log_dbg(...)    do{ fprintf(stdout, __VA_ARGS__); } while(0)