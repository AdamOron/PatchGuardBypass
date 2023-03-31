#include "Log.h"
#include <stdarg.h>
#include <ntddk.h>

ULONG
Log(
    PCSTR format,
    ...
)
{
    va_list args;
    va_start(args, format);
    ULONG result = vDbgPrintExWithPrefix("[+] ", DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, format, args);
    va_end(args);
    return result;
}
