#pragma once
#include <ntifs.h>
#include <ntddk.h>
#include "utils/log/Log.h"
#include "core/PatchGuard.h"

VOID
DriverUnload(
    _In_ PDRIVER_OBJECT pDriverObject
)
{
    UNREFERENCED_PARAMETER(pDriverObject);
}

#include "core/symbols/Globals.h"

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

    Globals::Initialize();

    PG::Disable::Execute();

    Log("%p\n", Globals::Functions::CcBcbProfiler);

    return STATUS_SUCCESS;
}
