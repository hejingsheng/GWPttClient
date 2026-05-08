#pragma once

#define GW_LOG_LEVEL_VERBOSE    0
#define GW_LOG_LEVEL_DEBUG      1
#define GW_LOG_LEVEL_INFO       2
#define GW_LOG_LEVEL_WARN       3
#define GW_LOG_LEVEL_ERROR      4
#define GW_LOG_LEVEL_NEVER      5

void print_log(const char *fmt, ...);

extern int gwlog_level;
#define GWLOG_PRINT(level, fmt, ...)       \
    do                                     \
    {                                      \
        if(level >= gwlog_level)             \
        {                                  \
            print_log(fmt, ##__VA_ARGS__); \
        }                                  \
    }while(0);  