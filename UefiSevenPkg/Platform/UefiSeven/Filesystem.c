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

#include "Filesystem.h"
#include "Util.h"


/**
  -----------------------------------------------------------------------------
  Exported method implementations.
  -----------------------------------------------------------------------------
**/


EFI_FILE_INFO *
GetFileInfo (
  IN  EFI_FILE_HANDLE    FileHandle
  )
{
  EFI_FILE_INFO   *FileInfo;
  UINTN           FileInfoSize;
  EFI_STATUS      Status;

  FileInfo      = NULL;
  FileInfoSize  = 0;
  Status        = FileHandle->GetInfo (FileHandle, &gEfiFileInfoGuid, &FileInfoSize, FileInfo);

  if (Status == EFI_BUFFER_TOO_SMALL) {
    FileInfo = AllocatePool (FileInfoSize);
    if (FileInfo == NULL) {
      Status = EFI_OUT_OF_RESOURCES;
    } else {
      Status = FileHandle->GetInfo (FileHandle, &gEfiFileInfoGuid, &FileInfoSize, FileInfo);
    }
  }

  if (EFI_ERROR (Status)) {
    if (FileInfo != NULL) {
      FreePool (FileInfo);
      FileInfo = NULL;
    }
  }

  return FileInfo;
}


/**
  Creates a new string representing path to a file identical
  to the one specified as input but with a different extension.
  Extension lengths do not have to match, but have to be greater
  than zero.

  @param[in] FilePath     Pointer to a string representing a file
                          path that will be the base for the
                          new path.

  @param[in] NewExtension Pointer to a string representing new
                          file extension that will be swapped
                          for the original one.

  @param[out] NewFilePath Pointer to a memory location storing
                          a the location of the first character
                          in the resultant string.

  @retval EFI_SUCCESS     No problems were encountered during
                          execution.
  @retval other           The operation failed.

**/
EFI_STATUS
ChangeExtension (
  IN  CHAR16  *FilePath,
  IN  CHAR16  *NewExtension,
  OUT VOID    **NewFilePath
  )
{
  UINTN   DotIndex;
  UINTN   NewExtensionLen;

  if ((FilePath == NULL) || (NewExtension == NULL) || (NewFilePath == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  *NewFilePath = NULL;

  DotIndex = StrLen (FilePath) - 1;
  while ((FilePath[DotIndex] != L'.') && (DotIndex != 0)) {
    DotIndex--;
  }

  if (DotIndex == 0) {
    return EFI_INVALID_PARAMETER;
  }

  // Move index as we want to copy the '.' too.
  DotIndex++;

  NewExtensionLen = StrLen (NewExtension);

  // Allocate enough to hold the new extension and null terminator.
  *NewFilePath = (CHAR16 *)AllocateZeroPool ((DotIndex + NewExtensionLen + 1) * sizeof (CHAR16));
  if (*NewFilePath == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  // Copy the relevant strings and add the null terminator.
  CopyMem ((CHAR16 *)*NewFilePath, FilePath, (DotIndex) * sizeof (CHAR16));
  CopyMem (((CHAR16 *)*NewFilePath) + DotIndex, NewExtension, NewExtensionLen * sizeof (CHAR16));

  return EFI_SUCCESS;
}


UINTN
GetEndingSlashIndex (
  IN  CHAR16    *CurrentFilePath,
  IN  BOOLEAN   NoSlashPrefixed
  )
{
  UINTN   EndingSlashIndex;

  if (CurrentFilePath == NULL) {
    return 0;
  }

  EndingSlashIndex = StrLen (CurrentFilePath) - 1;
  while ((CurrentFilePath[EndingSlashIndex] != L'\\') && (EndingSlashIndex != 0)) {
    EndingSlashIndex--;
  }
  if (NoSlashPrefixed) {
    while ((CurrentFilePath[EndingSlashIndex] == L'\\') && (EndingSlashIndex != 0)) {
      EndingSlashIndex--;
    }
  }

  return EndingSlashIndex;
}


EFI_STATUS
GetFilenameInSameDirectory (
  IN  CHAR16  *CurrentFilePath,
  IN  CHAR16  *NewFileName,
  OUT VOID    **NewFilePath
  )
{
  UINTN   EndingSlashIndex;
  UINTN   NewFileNameLen;

  if ((CurrentFilePath == NULL) || (NewFileName == NULL) || (NewFilePath == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  *NewFilePath = NULL;

  EndingSlashIndex = GetEndingSlashIndex (CurrentFilePath, FALSE);

  if ((EndingSlashIndex == 0) && (CurrentFilePath[EndingSlashIndex] != L'\\')) {
    return EFI_INVALID_PARAMETER;
  }

  EndingSlashIndex++;

  NewFileNameLen = StrLen (NewFileName);

  *NewFilePath = (CHAR16 *)AllocateZeroPool ((EndingSlashIndex + NewFileNameLen + 1) * sizeof (CHAR16));
  if (*NewFilePath == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  CopyMem ((CHAR16 *)*NewFilePath, CurrentFilePath, (EndingSlashIndex) * sizeof (CHAR16));
  CopyMem (((CHAR16 *)*NewFilePath) + EndingSlashIndex, NewFileName, NewFileNameLen * sizeof (CHAR16));

  return EFI_SUCCESS;
}


CHAR16 *
GetBaseFilename (
  IN  CHAR16  *CurrentFilePath
  )
{
  INTN  EndingSlashIndex;

  if (CurrentFilePath == NULL) {
    return NULL;
  }

  EndingSlashIndex = GetEndingSlashIndex (CurrentFilePath, TRUE);

  if (EndingSlashIndex < 0) {
    return NULL;
  }

  return &CurrentFilePath[EndingSlashIndex];
}


/**
  Checks whether a file located at a specified path exists on the
  filesystem where the UefiSeven executable is located.

  Any error messages will only be printed on the debug console
  and only the error code returned to caller.

  @param[in] FilePath     Pointer to a string representing a file
                          path whose existence will be checked.

  @retval TRUE            File exists at the specified location.
  @retval FALSE           File does not exist or other problems
                          were encountered during execution.

**/
BOOLEAN
FileExists (
  IN  EFI_FILE_HANDLE   VolumeRoot,
  IN  CHAR16            *FilePath
  )
{
  EFI_STATUS        Status;
  EFI_FILE_HANDLE   RequestedFile;

  if ((VolumeRoot == NULL) || (FilePath == NULL)) {
    return FALSE;
  }

  // Try to open file for reading.
  Status = VolumeRoot->Open (VolumeRoot, &RequestedFile, FilePath, EFI_FILE_MODE_READ, 0);
  if (EFI_ERROR (Status)) {
    PrintDebug (L"Unable to open file '%s' for reading (error: %r)\n", FilePath, Status);
    return FALSE;
  } else {
    PrintDebug (L"Opened file '%s' for reading\n", FilePath);
    RequestedFile->Close (RequestedFile);
    return TRUE;
  }
}

BOOLEAN
FileDelete (
  IN  EFI_FILE_HANDLE   VolumeRoot,
  IN  CHAR16            *FilePath
  )
{
  EFI_STATUS        Status;
  EFI_FILE_HANDLE   RequestedFile;
  EFI_FILE_INFO     *FileInfo;

  if ((VolumeRoot == NULL) || (FilePath == NULL)) {
    return FALSE;
  }

  // Try to open file for deleting.
  Status = VolumeRoot->Open (VolumeRoot, &RequestedFile, FilePath, EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, 0);
  if (EFI_ERROR (Status)) {
    PrintDebug (L"Unable to open file '%s' for deleting (error: %r)\n", FilePath, Status);
    return FALSE;
  } else {
    PrintDebug (L"Opened file '%s' for deleting\n", FilePath);
    FileInfo = GetFileInfo (RequestedFile);
    if (FileInfo != NULL) {
      // Delete if its not directory.
      if ((FileInfo->Attribute & EFI_FILE_DIRECTORY) == 0) {
        RequestedFile->Delete (RequestedFile);
      }
      FreePool (FileInfo);
    }
    return TRUE;
  }
}

BOOLEAN
FileWrite (
  IN  EFI_FILE_HANDLE   VolumeRoot,
  IN  CHAR16            *FilePath,
  IN  UINT8             *Buffer,
  IN  UINTN             BufferSize
  )
{
  EFI_STATUS        Status;
  EFI_FILE_HANDLE   RequestedFile;
  EFI_FILE_INFO     *FileInfo;

  if ((VolumeRoot == NULL) || (FilePath == NULL) || (Buffer == NULL) || (BufferSize == 0)) {
    return FALSE;
  }

  // Try to open file for writing.
  Status = VolumeRoot->Open (VolumeRoot, &RequestedFile, FilePath, EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, 0);
  if (EFI_ERROR (Status)) {
    PrintDebug (L"Unable to open file '%s' for writing (error: %r)\n", FilePath, Status);
    return FALSE;
  } else {
    PrintDebug (L"Opened file '%s' for writing\n", FilePath);
    FileInfo = GetFileInfo (RequestedFile);
    if (FileInfo != NULL) {
      // Write if its not directory.
      if ((FileInfo->Attribute & EFI_FILE_DIRECTORY) == 0) {
        RequestedFile->Write (RequestedFile, &BufferSize, Buffer);
        RequestedFile->Close (RequestedFile);
      }
      FreePool (FileInfo);
    }
    return TRUE;
  }
}


/**
  Reads a file located at a specified path on the filesystem
  where the UefiSeven executable is located into a buffer.

  Any error messages will only be printed on the debug console
  and only the error code returned to caller.

  @param[in] FilePath       Pointer to a string representing a file
                            path that will be the base for the
                            new path.

  @param[out] FileContents  Pointer to a memory location string
                            the contents of the file if no problems
                            were encountered over the course of
                            execution; NULL otherwise.

  @param[out] FileBytes     Number of bytes read off the disk if
                            no problems were encountered over
                            the course of execution; NULL otherwise.

  @retval EFI_SUCCESS       No problems were encountered over the
                            course of execution.
  @retval other             The operation failed.

**/
EFI_STATUS
FileRead (
  IN  EFI_FILE_HANDLE   VolumeRoot,
  IN  CHAR16            *FilePath,
  OUT VOID              **FileContents,
  OUT UINTN             *FileBytes
  )
{
  EFI_STATUS              Status;
  EFI_FILE_HANDLE         File = NULL;
  EFI_FILE_INFO           *FileInfo;
  UINTN                   Size;

  if ((VolumeRoot == NULL) || (FilePath == NULL) || (FileContents == NULL) || (FileBytes == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  // Try to open file for reading.
  Status = VolumeRoot->Open (VolumeRoot, &File, FilePath, EFI_FILE_MODE_READ, 0);
  if (EFI_ERROR (Status)) {
    PrintDebug (L"Unable to open file '%s' for reading (error: %r)\n", FilePath, Status);
    goto Exit;
  } else {
    PrintDebug (L"Opened file '%s' for reading\n", FilePath);
  }

  // First gather information on total file size.
  FileInfo = GetFileInfo (File);
  if (FileInfo == NULL) {
    Status = EFI_UNSUPPORTED;
    goto Exit;
  }
  Size = FileInfo->FileSize;
  FreePool (FileInfo);

  // Allocate a buffer...
  *FileContents = AllocatePool (Size);
  if (*FileContents == NULL) {
    PrintDebug (L"Unable to allocate %u bytes for file contents\n", Size);
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  } else {
    PrintDebug (L"Allocated %u bytes for file contents\n", Size);
  }

  // ... and read the entire file into it.
  Status = File->Read (File, &Size, *FileContents);
  if (EFI_ERROR (Status)) {
    PrintDebug (L"Unable to read file contents (error: %r)\n", Status);
    goto Exit;
  } else {
    PrintDebug (L"Read file contents success\n");
    *FileBytes = Size;
  }

  Exit:

  // Cleanup.
  if (EFI_ERROR (Status) && (*FileContents != NULL)) {
    FreePool (*FileContents);
    *FileContents = NULL;
  }
  if (File != NULL) {
    File->Close (File);
  }

  return Status;
}


EFI_STATUS
CheckBootMgrGuid (
  IN UINT8  *ImageBase,
  IN UINTN  ImageSize
  )
{
  EFI_STATUS  Status;
  UINT8       *Address;

  //
  // EfiGuard: Check for the BCD Bootmgr GUID, { 9DEA862C-5CDD-4E70-ACC1-F32B344D4795 },
  //           which is present in bootmgfw/bootmgr (and on Win >= 8 also winload.[exe|efi])
  //

  Status = EFI_UNSUPPORTED;

  if ((ImageBase == NULL) || (ImageSize == 0)) {
    return Status;
  }

  for (Address = ImageBase;
    Address < ImageBase + ImageSize - sizeof (EFI_GUID);
    Address += sizeof (VOID *)
    )
  {
    if (CompareGuid ((CONST EFI_GUID *)Address, &gBcdWindowsBootmgrGuid)) {
      PrintDebug (L"Found %g\n", &gBcdWindowsBootmgrGuid);
      Status = EFI_SUCCESS;
      break;
    }
  }

  return Status;
}


EFI_STATUS
Launch (
  IN  CHAR16  *FilePath,
  IN  VOID    (*WaitForEnterCallback) (BOOLEAN)
  )
{
  EFI_STATUS                  Status;
  EFI_DEVICE_PATH_PROTOCOL    *FilePathOnDevice;
  EFI_HANDLE                  FileImageHandle;
  EFI_LOADED_IMAGE_PROTOCOL   *FileImageInfo;
  CHAR16                      *FilePathOnDeviceText;

  if ((FilePath == NULL) || (mUefiSevenImageInfo == NULL) || (mUefiSevenImage == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Try to load the image first.
  //
  FilePathOnDevice      = FileDevicePath (mUefiSevenImageInfo->DeviceHandle, FilePath);
  FilePathOnDeviceText  = ConvertDevicePathToText (FilePathOnDevice, TRUE, FALSE);
  Status = gBS->LoadImage (TRUE, mUefiSevenImage, FilePathOnDevice, NULL, 0, &FileImageHandle);
  if (EFI_ERROR (Status)) {
    PrintError (L"Unable to load '%s' (error: %r)\n", FilePathOnDeviceText, Status);
  } else {
    PrintDebug (L"Loaded '%s'\n", FilePathOnDeviceText);
    PrintDebug (L"Addresss behind FileImageHandle=%x\n", FileImageHandle);
  }
  if (FilePathOnDeviceText != NULL) {
    FreePool (FilePathOnDeviceText);
  }

  //
  // Make sure this is a valid EFI loader and fill in the options.
  //
  Status = gBS->HandleProtocol (FileImageHandle, &gEfiLoadedImageProtocolGuid, (VOID *)&FileImageInfo);
  if (EFI_ERROR (Status)
    || (FileImageInfo->ImageCodeType != EfiLoaderCode)
    || EFI_ERROR (CheckBootMgrGuid ((UINT8 *)FileImageInfo->ImageBase, FileImageInfo->ImageSize))
    )
  {
    PrintError (L"File does not match an EFI loader signature\n");
    gBS->UnloadImage (FileImageHandle);
    return EFI_UNSUPPORTED;
  } else {
    PrintDebug (L"File matches an EFI loader signature\n");
  }

  if (WaitForEnterCallback != NULL) {
    WaitForEnterCallback (TRUE);
  }

  //
  // Launch!
  //
  Status = gBS->StartImage (FileImageHandle, NULL, NULL);
  if (EFI_ERROR (Status)) {
    PrintError (L"Unable to start image (error: %r)\n", Status);
  }

  return Status;
}
