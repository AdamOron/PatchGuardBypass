#pragma once
#include <ntifs.h>
#include <ntddk.h>
#include "Log.h"
#include "PatchGuard.h"

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

    PG::Disable::Execute();

    return STATUS_SUCCESS;
}
