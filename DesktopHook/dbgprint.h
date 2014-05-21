
#ifndef DBGPRINT_H__
#define DBGPRINT_H__

void debug_print(TCHAR *format, ...);

#ifndef NO_PRINT
    #define DbgPrint(format, ...) debug_print(format, __VA_ARGS__)
#else
    #define DbgPrint(format, ...)
#endif

#endif