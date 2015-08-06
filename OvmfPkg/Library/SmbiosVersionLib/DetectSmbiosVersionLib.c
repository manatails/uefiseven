/** @file

  A hook-in library for MdeModulePkg/Universal/SmbiosDxe, in order to set
  gEfiMdeModulePkgTokenSpaceGuid.PcdSmbiosVersion (and possibly other PCDs)
  just before SmbiosDxe consumes them.

  Copyright (C) 2013, 2015, Red Hat, Inc.
  Copyright (c) 2008 - 2012, Intel Corporation. All rights reserved.<BR>

  This program and the accompanying materials are licensed and made available
  under the terms and conditions of the BSD License which accompanies this
  distribution. The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS, WITHOUT
  WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <IndustryStandard/SmBios.h>

#include <Base.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/QemuFwCfgLib.h>

typedef union {
  SMBIOS_TABLE_ENTRY_POINT     V2;
} QEMU_SMBIOS_ANCHOR;

RETURN_STATUS
EFIAPI
DetectSmbiosVersion (
  VOID
  )
{
  FIRMWARE_CONFIG_ITEM Anchor, Tables;
  UINTN                AnchorSize, TablesSize;
  QEMU_SMBIOS_ANCHOR   QemuAnchor;
  UINT16               SmbiosVersion;

  if (RETURN_ERROR (QemuFwCfgFindFile (
                      "etc/smbios/smbios-anchor", &Anchor, &AnchorSize)) ||
      RETURN_ERROR (QemuFwCfgFindFile (
                      "etc/smbios/smbios-tables", &Tables, &TablesSize)) ||
      TablesSize == 0) {
    return RETURN_SUCCESS;
  }

  QemuFwCfgSelectItem (Anchor);

  switch (AnchorSize) {
  case sizeof QemuAnchor.V2:
    QemuFwCfgReadBytes (AnchorSize, &QemuAnchor);

    if (QemuAnchor.V2.MajorVersion != 2 ||
        QemuAnchor.V2.TableLength != TablesSize ||
        CompareMem (QemuAnchor.V2.AnchorString, "_SM_", 4) != 0 ||
        CompareMem (QemuAnchor.V2.IntermediateAnchorString, "_DMI_", 5) != 0) {
      return RETURN_SUCCESS;
    }
    SmbiosVersion = (UINT16)(QemuAnchor.V2.MajorVersion << 8 |
                             QemuAnchor.V2.MinorVersion);
    break;

  default:
    return RETURN_SUCCESS;
  }

  DEBUG ((EFI_D_INFO, "%a: SMBIOS version from QEMU: 0x%04x\n", __FUNCTION__,
    SmbiosVersion));
  PcdSet16 (PcdSmbiosVersion, SmbiosVersion);

  return RETURN_SUCCESS;
}