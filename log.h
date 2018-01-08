#ifndef LOG_H
#define LOG_H

#define _LOG(...)  fprintf(stderr, __VA_ARGS__)
#define LOG(fmt,...) _LOG(fmt "\n", ##__VA_ARGS__)

#endif // LOG_H
