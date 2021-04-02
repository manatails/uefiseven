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

#ifndef __FILESYSTEM_H
#define __FILESYSTEM_H


/**
  -----------------------------------------------------------------------------
  Includes.
  -----------------------------------------------------------------------------
**/

#include <Uefi.h>

#include <Guid/FileInfo.h>

#include <Protocol/DevicePath.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleFileSystem.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DevicePathLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>


/**
  -----------------------------------------------------------------------------
  Exported method signatures.
  -----------------------------------------------------------------------------
**/

EFI_FILE_INFO *
GetFileInfo (
  IN  EFI_FILE_HANDLE    FileHandle
  );

BOOLEAN
FileExists (
  IN  EFI_FILE_HANDLE   VolumeRoot,
  IN  CHAR16            *FilePath
  );

EFI_STATUS
ChangeExtension (
  IN  CHAR16  *FilePath,
  IN  CHAR16  *NewExtension,
  OUT VOID    **NewFilePath
  );

EFI_STATUS
GetFilenameInSameDirectory (
  IN  CHAR16  *CurrentFilePath,
  IN  CHAR16  *NewFileName,
  OUT VOID    **NewFilePath
  );

CHAR16 *
GetBaseFilename (
  IN  CHAR16  *CurrentFilePath
  );

EFI_STATUS
FileRead (
  IN  EFI_FILE_HANDLE   VolumeRoot,
  IN  CHAR16            *FilePath,
  OUT VOID              **FileContents,
  OUT UINTN             *FileBytes
  );

EFI_STATUS
Launch (
  IN  CHAR16  *FilePath,
  IN  VOID    (*WaitForEnterCallback) (BOOLEAN)
  );

BOOLEAN
FileDelete (
  IN  EFI_FILE_HANDLE   VolumeRoot,
  IN  CHAR16            *FilePath
  );

BOOLEAN
FileWrite (
  IN  EFI_FILE_HANDLE   VolumeRoot,
  IN  CHAR16            *FilePath,
  IN  UINT8             *Buffer,
  IN  UINTN             BufferSize
  );


/**
  -----------------------------------------------------------------------------
  Imported global variables.
  -----------------------------------------------------------------------------
**/

extern  EFI_HANDLE                  mUefiSevenImage;
extern  EFI_LOADED_IMAGE_PROTOCOL   *mUefiSevenImageInfo;


#endif
