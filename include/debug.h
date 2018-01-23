#ifndef __DEBUG_H__
#define __DEBUG_H__

#include "log_manager.h"

#if 0
#define DBG_INFO(...)   debuglog("INFO", __FUNCTION__, __LINE__, __VA_ARGS__) 
#define DBG_WARN(...)   debuglog("WARN", __FUNCTION__, __LINE__, __VA_ARGS__) 
#define DBG_ERROR(...)  debuglog("ERROR", __FUNCTION__, __LINE__, __VA_ARGS__) 
#else
#define DBG_INFO(...)   log_print(__VA_ARGS__)
#define DBG_WARN(...)   debuglog("WARN", __FUNCTION__, __LINE__, __VA_ARGS__) 
#define DBG_ERROR(...)  debuglog("ERROR", __FUNCTION__, __LINE__, __VA_ARGS__) 
#endif //

void debuglog(const char *msg, const char *func, const int line, const char *fmt, ...);

#endif //__DEBUG_H__

