#include "Globals.h"

/*
Just grouping together all offsets into a single namespace.
Not necessary, but more convenient for modifying and hiding the vars from global scope.
*/
namespace GlobalOffsets
{
	/* Keys used for decoding/encoding DPCs within KTIMERs */
	constexpr UINT64 KiWaitAlways = 0xCFC9F8;
	constexpr UINT64 KiWaitNever = 0xCFC7F8;
	/* Number of processor on the system. Also indicates number of PRCBs */
	constexpr UINT64 KeNumberProcessors_0 = 0xCFC404;
	constexpr UINT64 HalpClockTimer = 0xC4BFC8;

	/* Offset of some known routine that we can use to calculate ImageBase */
	constexpr UINT64 KeBugCheckEx = 0x3f5210;
	/* Undocumented function that returns the Prcb at the given index */
	constexpr UINT64 KeGetPrcb = 0x32DAD0;
	constexpr UINT64 CcBcbProfiler = 0x3D7330;
};

/**
Template function to return a Global variable.
@param Type is the type of the variable.
@param GlobalOffset is the offset of the variable from the image base.
*/
template <typename Type, UINT64 GlobalOffset>
Type GetGlobal(UINT64 ImageBase)
{
	return reinterpret_cast<Type>(ImageBase + GlobalOffset);
}

/**
Initializes all Globals.
*/
BOOLEAN
Globals::Initialize(
	VOID
)
{
	/* Not sure if this is ok or completely mentally ill */
	UINT64 ImageBase = (UINT64) &KeBugCheckEx - GlobalOffsets::KeBugCheckEx;

	Globals::Variables::KiWaitAlways = GetGlobal<PINT64, GlobalOffsets::KiWaitAlways>(ImageBase);
	Globals::Variables::KiWaitNever = GetGlobal<PINT64, GlobalOffsets::KiWaitNever>(ImageBase);
	Globals::Variables::KeNumberProcessors_0 = GetGlobal<PULONG, GlobalOffsets::KeNumberProcessors_0>(ImageBase);
	Globals::Variables::HalpClockTimer = GetGlobal<PVOID *, GlobalOffsets::HalpClockTimer>(ImageBase);

	Globals::Functions::KeGetPrcb = GetGlobal<PKPRCB (__fastcall *) (ULONG), GlobalOffsets::KeGetPrcb>(ImageBase);
	Globals::Functions::CcBcbProfiler = GetGlobal<PVOID, GlobalOffsets::CcBcbProfiler>(ImageBase);

	return TRUE;
}

/* Define all externs */
PINT64 Globals::Variables::KiWaitAlways = NULL;
PINT64 Globals::Variables::KiWaitNever = NULL;
PULONG Globals::Variables::KeNumberProcessors_0 = NULL;
PVOID *Globals::Variables::HalpClockTimer = NULL;
PKPRCB (__fastcall *Globals::Functions::KeGetPrcb) (ULONG) = NULL;
PVOID Globals::Functions::CcBcbProfiler = NULL;
