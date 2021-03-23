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
ChangeExtension(
	IN	CHAR16	*FilePath,
	IN	CHAR16	*NewExtension,
	OUT	VOID	**NewFilePath)
{
	UINTN	DotIndex;
	UINTN	NewExtensionLen = StrLen(NewExtension);

	*NewFilePath = 0;
	DotIndex = StrLen(FilePath) - 1;
	while ((FilePath[DotIndex] != L'.') && (DotIndex != 0)) {
		DotIndex--;
	}

	if (DotIndex == 0)
		return EFI_INVALID_PARAMETER;

	// Move index as we want to copy the '.' too.
	DotIndex++;

	// Allocate enough to hold the new extension and null terminator.
	*NewFilePath = (CHAR16*)AllocateZeroPool((DotIndex + NewExtensionLen + 1) * sizeof(CHAR16));
	if (*NewFilePath == NULL)
		return EFI_OUT_OF_RESOURCES;
	
	// Copy the relevant strings and add the null terminator.
	CopyMem((CHAR16 *)*NewFilePath, FilePath, (DotIndex) * sizeof(CHAR16));
	CopyMem(((CHAR16 *)*NewFilePath) + DotIndex, NewExtension, NewExtensionLen * sizeof(CHAR16));
	
	return EFI_SUCCESS;
}

UINTN
GetEndingSlashIndex (
	IN	CHAR16	*CurrentFilePath,
	IN	BOOLEAN	NoSlashPrefixed)
{
	UINTN	EndingSlashIndex;

	EndingSlashIndex = StrLen(CurrentFilePath) - 1;
	while ((CurrentFilePath[EndingSlashIndex] != L'\\') && (EndingSlashIndex != 0)) {
		EndingSlashIndex--;
	}
	if (NoSlash) {
		while ((CurrentFilePath[EndingSlashIndex] == L'\\') && (EndingSlashIndex != 0)) {
			EndingSlashIndex--;
		}
	}

	return EndingSlashIndex;
}

EFI_STATUS
GetFilenameInSameDirectory(
	IN	CHAR16	*CurrentFilePath,
	IN	CHAR16	*NewFileName,
	OUT	VOID	**NewFilePath)
{
	UINTN	EndingSlashIndex;
	UINTN	NewFileNameLen = StrLen(NewFileName);

	*NewFilePath = 0;

	EndingSlashIndex = GetEndingSlashIndex (CurrentFilePath, FALSE);

	if (EndingSlashIndex == 0 && (CurrentFilePath[EndingSlashIndex] != L'\\'))
		return EFI_INVALID_PARAMETER;

	EndingSlashIndex++;

	*NewFilePath = (CHAR16*)AllocateZeroPool((EndingSlashIndex + NewFileNameLen + 1) * sizeof(CHAR16));
	if (*NewFilePath == NULL)
		return EFI_OUT_OF_RESOURCES;

	CopyMem((CHAR16 *)*NewFilePath, CurrentFilePath, (EndingSlashIndex) * sizeof(CHAR16));
	CopyMem(((CHAR16 *)*NewFilePath) + EndingSlashIndex, NewFileName, NewFileNameLen * sizeof(CHAR16));
	
	return EFI_SUCCESS;
}

CHAR16 *
GetBaseFilename(
	IN	CHAR16	*CurrentFilePath)
{
	INTN	EndingSlashIndex;

	EndingSlashIndex = GetEndingSlashIndex (CurrentFilePath, TRUE);

	if (EndingSlashIndex < 0)
		return NULL;

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
FileExists(
	IN	CHAR16*	FilePath)
{
	EFI_STATUS				Status;
	EFI_FILE_IO_INTERFACE	*Volume;
	EFI_FILE_HANDLE			VolumeRoot;
	EFI_FILE_HANDLE			RequestedFile;

	// Open volume where UefiSeven resides.
	Status = gBS->HandleProtocol(UefiSevenImageInfo->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void **)&Volume);
	if (EFI_ERROR(Status)) {
		PrintDebug(L"Unable to find simple file system protocol (error: %r)\n", Status);
		return FALSE;
	} else {
		PrintDebug(L"Found simple file system protocol\n");
	}
	Status = Volume->OpenVolume(Volume, &VolumeRoot);
	if (EFI_ERROR(Status)) {
		PrintDebug(L"Unable to open volume (error: %r)\n", Status);
		return FALSE;
	} else {
		PrintDebug(L"Opened volume\n");
	}

	// Try to open file for reading.
	Status = VolumeRoot->Open(VolumeRoot, &RequestedFile, FilePath, EFI_FILE_MODE_READ, 0);
	if (EFI_ERROR(Status)) {
		PrintDebug(L"Unable to open file '%s' for reading (error: %r)\n", FilePath, Status);
		VolumeRoot->Close(VolumeRoot);
		return FALSE;
	} else {
		PrintDebug(L"Opened file '%s' for reading\n", FilePath);
		RequestedFile->Close(RequestedFile);
		VolumeRoot->Close(VolumeRoot);
		return TRUE;
	}
}


/**
  Reads a file located at a specified path on the filesystem 
  where the UefiSeven executable is located into a buffer.

  Any error messages will only be printed on the debug console
  and only the error code returned to caller.

  @param[in] FilePath      Pointer to a string representing a file
                           path that will be the base for the
						   new path.

  @param[out] FileContents Pointer to a memory location string
                           the contents of the file if no problems
						   were encountered over the course of
						   execution; NULL otherwise.

  @param[out] FileBytes    Number of bytes read off the disk if
                           no problems were encountered over
						   the course of execution; NULL otherwise.

  @retval EFI_SUCCESS      No problems were encountered over the
                           course of execution.
  @retval other            The operation failed.
  
**/
EFI_STATUS
FileRead(
	IN	CHAR16	*FilePath,
	OUT	VOID	**FileContents,
	OUT	UINTN	*FileBytes)
{
	EFI_STATUS				Status;
	EFI_FILE_IO_INTERFACE	*Volume;
	EFI_FILE_HANDLE			VolumeRoot = NULL;
	EFI_FILE_HANDLE			File = NULL;
	EFI_FILE_INFO			*FileInfo;
	UINTN					Size;

	// Open volume where UefiSeven resides.
	Status = gBS->HandleProtocol(UefiSevenImageInfo->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void **)&Volume);
	if (EFI_ERROR(Status)) {
		PrintDebug(L"Unable to find simple file system protocol (error: %r)\n", Status);
		goto Exit;
	} else {
		PrintDebug(L"Found simple file system protocol\n");
	}
		
	Status = Volume->OpenVolume(Volume, &VolumeRoot);
	if (EFI_ERROR(Status)) {
		PrintDebug(L"Unable to open volume (error: %r)\n", Status);
		goto Exit;
	} else {
		PrintDebug(L"Opened volume\n");
	}
	
	// Try to open file for reading.
	Status = VolumeRoot->Open(VolumeRoot, &File, FilePath, EFI_FILE_MODE_READ, 0);
	if (EFI_ERROR(Status)) {
		PrintDebug(L"Unable to open file '%s' for reading (error: %r)\n", FilePath, Status);
		goto Exit;
	} else {
		PrintDebug(L"Opened file '%s' for reading\n", FilePath);
	}

	// First gather information on total file size.
	File->GetInfo(File, &gEfiFileInfoGuid, &Size, NULL);
	FileInfo = AllocatePool(Size);
	if (FileInfo == NULL) {
		PrintDebug(L"Unable to allocate %u bytes for file info\n", Size);
		Status = EFI_OUT_OF_RESOURCES;
		goto Exit;
	} else {
		PrintDebug(L"Allocated %u bytes for file info\n", Size);
	}
	File->GetInfo(File, &gEfiFileInfoGuid, &Size, FileInfo);
	Size = FileInfo->FileSize;
	FreePool(FileInfo);

	// Allocate a buffer...
	*FileContents = AllocatePool(Size);
	if (*FileContents == NULL) {
		PrintDebug(L"Unable to allocate %u bytes for file contents\n", Size);
		Status = EFI_OUT_OF_RESOURCES;
		goto Exit;
	} else {
		PrintDebug(L"Allocated %u bytes for file contents\n", Size);
	}

	// ... and read the entire file into it.
	Status = File->Read(File, &Size, *FileContents);
	if (EFI_ERROR(Status)) {
		PrintDebug(L"Unable to read file contents (error: %r)\n", Status);
		goto Exit;
	} else {
		PrintDebug(L"Read file contents\n", Status);
		*FileBytes = Size;
	}

Exit:
	// Cleanup.
	if (EFI_ERROR(Status) && *FileContents != NULL) {
		FreePool(*FileContents);
		*FileContents = NULL;
	}
	if (File != NULL)
		File->Close(File);
	if (VolumeRoot != NULL)
		VolumeRoot->Close(VolumeRoot);
	return Status;
}

EFI_STATUS
CheckBootMgrGuid (
	IN UINT8* ImageBase,
	IN UINTN ImageSize
	)
{
	EFI_STATUS			Status;
	UINT8 					*Address;
	CONST EFI_GUID 	BcdWindowsBootmgrGuid = { 0x9dea862c, 0x5cdd, 0x4e70, { 0xac, 0xc1, 0xf3, 0x2b, 0x34, 0x4d, 0x47, 0x95 } };

	//
	// EfiGuard: Check for the BCD Bootmgr GUID, { 9DEA862C-5CDD-4E70-ACC1-F32B344D4795 },
	// 					 which is present in bootmgfw/bootmgr (and on Win >= 8 also winload.[exe|efi])
	//

	Status = EFI_UNSUPPORTED;

	for (Address = ImageBase; Address < ImageBase + ImageSize - sizeof(BcdWindowsBootmgrGuid); Address += sizeof(VOID*))
	{
		if (CompareGuid((CONST GUID*)Address, &BcdWindowsBootmgrGuid))
		{
			Status = EFI_SUCCESS;
			break;
		}
	}

	PrintDebug(L"CheckBootMgrGuid - %r\n", Status);

	return Status;
}

EFI_STATUS
Launch(
	IN	CHAR16	*FilePath,
	IN	VOID	(*WaitForEnterCallback)(BOOLEAN))
{
	EFI_STATUS					Status;
	EFI_DEVICE_PATH_PROTOCOL	*FilePathOnDevice;
	EFI_HANDLE					FileImageHandle;
	EFI_LOADED_IMAGE_PROTOCOL	*FileImageInfo;
	CHAR16						*FilePathOnDeviceText;

	//
	// Try to load the image first.
	//
	FilePathOnDevice = FileDevicePath(UefiSevenImageInfo->DeviceHandle, FilePath);
	FilePathOnDeviceText = ConvertDevicePathToText(FilePathOnDevice, TRUE, FALSE);
	Status = gBS->LoadImage(TRUE, UefiSevenImage, FilePathOnDevice, NULL, 0, &FileImageHandle);
	if (EFI_ERROR(Status)) {
		PrintError(L"Unable to load '%s' (error: %r)\n", FilePathOnDeviceText, Status);
	} else {
		PrintDebug(L"Loaded '%s'\n", FilePathOnDeviceText);
		PrintDebug(L"Addresss behind FileImageHandle=%x\n", FileImageHandle);
	}
	FreePool(FilePathOnDeviceText);
	
	// 
	// Make sure this is a valid EFI loader and fill in the options.
	//
	Status = gBS->HandleProtocol(FileImageHandle, &gEfiLoadedImageProtocolGuid, (VOID *)&FileImageInfo);
	if (EFI_ERROR(Status) || FileImageInfo->ImageCodeType != EfiLoaderCode || EFI_ERROR(CheckBootMgrGuid((UINT8*)FileImageInfo->ImageBase, FileImageInfo->ImageSize))) {
		PrintError(L"File does not match an EFI loader signature\n");
		gBS->UnloadImage(FileImageHandle);
		return EFI_UNSUPPORTED;
	} else {
		PrintDebug(L"File matches an EFI loader signature\n");
	}

	if (WaitForEnterCallback != NULL) {
		WaitForEnterCallback(TRUE);
	}
	
	//
	// Launch!
	//
	Status = gBS->StartImage(FileImageHandle, NULL, NULL);
	if (EFI_ERROR(Status)) {
		PrintError(L"Unable to start image (error: %r)\n", Status);
	}

	return Status;
}
