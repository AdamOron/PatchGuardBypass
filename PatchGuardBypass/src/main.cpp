#pragma once
#include <ntifs.h>
#include <ntddk.h>
#include "Log.h"
#include "timers/Timer.h"

VOID
DriverUnload(
    _In_ PDRIVER_OBJECT pDriverObject
)
{
    UNREFERENCED_PARAMETER(pDriverObject);
}

VOID
PgDisabler(
    PKTIMER Timer,
    PKDPC DecodedDpc
)
{
    if (!MmIsAddressValid(DecodedDpc))
        return;

    INT64 ShiftedCtx = (INT64) DecodedDpc->DeferredContext >> 47;

    if (ShiftedCtx != 0 && ShiftedCtx != -1)
    {
        DbgBreakPoint();
        Log("\nContext-Aware Timer/DPC: %p/%p\n", Timer, DecodedDpc);
    }

    if ((UINT64) DecodedDpc->DeferredRoutine == 0xfffff80323dd7330)
    {
        DbgBreakPoint();
        Log("\nContext-Unaware Timer/DPC: %p/%p\n", Timer, DecodedDpc);
    }
}

EXTERN_C
NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
)
{
    UNREFERENCED_PARAMETER(DriverObject);
    UNREFERENCED_PARAMETER(RegistryPath);

    DriverObject->DriverUnload = DriverUnload;

    TimerCallbackArray callbacks;
    callbacks.Append((PTIMER_CALLBACK) &PgDisabler);

    Log("Starting %d\n", callbacks.Size());

    IterateSystemTimers(callbacks);

    return STATUS_SUCCESS;
}
