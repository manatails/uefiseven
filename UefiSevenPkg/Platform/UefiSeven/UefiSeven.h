/** @file

  Copyright (c) 2020, Seungjoo Kim
  Copyright (c) 2016, Dawid Ciecierski

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __UEFI_SEVEN_H
#define __UEFI_SEVEN_H


/**
  -----------------------------------------------------------------------------
  Includes.
  -----------------------------------------------------------------------------
**/

#include <Uefi.h>

#include <Guid/FileInfo.h>

#include <IndustryStandard/LegacyVgaBios.h>

#include <Protocol/LegacyRegion.h>
#include <Protocol/LegacyRegion2.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleTextInEx.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/IniParsingLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/MtrrLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>


/**
  -----------------------------------------------------------------------------
  Type definitions and enums.
  -----------------------------------------------------------------------------
**/

#pragma pack(1)
typedef struct {
  UINT16  Offset;
  UINT16  Segment;
} IVT_ENTRY;
#pragma pack()

typedef enum {
  LOCK,
  UNLOCK
} MEMORY_LOCK_OPERATION;


/**
  -----------------------------------------------------------------------------
  Local method signatures.
  -----------------------------------------------------------------------------
**/

BOOLEAN
ShowAnimatedLogo (
  VOID
  );

BOOLEAN
CanWriteAtAddress (
  IN  EFI_PHYSICAL_ADDRESS    Address
  );

EFI_STATUS
EnsureMemoryLock (
  IN  EFI_PHYSICAL_ADDRESS    Address,
  IN  UINT32                  Length,
  IN  MEMORY_LOCK_OPERATION   Operation
  );

BOOLEAN
IsInt10hHandlerDefined (
  VOID
  );

EFI_STATUS
ShimVesaInformation (
  IN  EFI_PHYSICAL_ADDRESS    StartAddress,
  OUT EFI_PHYSICAL_ADDRESS    *EndAddress
  );

VOID
WaitForEnter (
  IN  BOOLEAN   PrintMessage
  );

VOID
WaitForEnterAndStall (
  IN  BOOLEAN   PrintMessage
  );

/**
  -----------------------------------------------------------------------------
  Constants.
  -----------------------------------------------------------------------------
**/

STATIC CONST  CHAR8                 VENDOR_NAME[]       = "UefiSeven";
STATIC CONST  CHAR8                 PRODUCT_NAME[]      = "Emulated VGA";
STATIC CONST  CHAR8                 PRODUCT_REVISION[]  = "OVMF Int10h (fake)";
STATIC CONST  EFI_PHYSICAL_ADDRESS  VGA_ROM_ADDRESS     = 0xC0000;
STATIC CONST  EFI_PHYSICAL_ADDRESS  IVT_ADDRESS         = 0x00000;
STATIC CONST  UINTN                 VGA_ROM_SIZE        = 0x10000;
STATIC CONST  UINTN                 FIXED_MTRR_SIZE     = 0x20000;


#endif
