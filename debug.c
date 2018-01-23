#include <stdarg.h>
#include <stdio.h>

void debuglog(const char *msg, const char *func, const int line, const char *fmt, ...)
{
    static char output[1024];
    static char sendbuf[1024];

    va_list arglst;
    unsigned short info_len = 0;

    va_start(arglst,fmt);
	info_len = vsnprintf(output,sizeof(output),fmt,arglst);
	va_end(arglst);

    info_len = snprintf(sendbuf, sizeof(sendbuf), "[%s][%s:%05d] %s", msg, func, line, output);
    printf("%s", sendbuf);

}

