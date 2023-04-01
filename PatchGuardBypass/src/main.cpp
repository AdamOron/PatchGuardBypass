#pragma once
#include <ntifs.h>
#include <ntddk.h>
#include "utils/Log.h"
#include "core/PatchGuard.h"

VOID
DriverUnload(
    _In_ PDRIVER_OBJECT pDriverObject
)
{
    UNREFERENCED_PARAMETER(pDriverObject);
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

    Log("Starting: %p\n", KeGetPcr()->CurrentPrcb);
    PG::Disable::Execute();

    return STATUS_SUCCESS;
}
