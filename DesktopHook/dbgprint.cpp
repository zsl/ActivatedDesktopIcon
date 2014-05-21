#include "stdafx.h"
#include "dbgprint.h"

void debug_print(TCHAR *format, ...)
{
    TCHAR buffer[256];
    va_list args;
    va_start(args, format);
    _vstprintf(buffer, format, args);
    va_end(args);

    ::OutputDebugString(buffer);
}