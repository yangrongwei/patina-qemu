/** @file

  A hook-in library for MdeModulePkg/Universal/SmbiosDxe, in order to set
  gEfiMdeModulePkgTokenSpaceGuid.PcdSmbiosVersion (and possibly other PCDs)
  just before SmbiosDxe consumes them.

  Copyright (C) 2013, 2015, Red Hat, Inc.
  Copyright (c) 2008 - 2012, Intel Corporation. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <IndustryStandard/SmBios.h>

#include <Base.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/QemuFwCfgLib.h>

typedef union {
  SMBIOS_TABLE_ENTRY_POINT        V2;
  SMBIOS_TABLE_3_0_ENTRY_POINT    V3;
} QEMU_SMBIOS_ANCHOR;

RETURN_STATUS
EFIAPI
DetectSmbiosVersion (
  VOID
  )
{
  FIRMWARE_CONFIG_ITEM  Anchor, Tables;
  UINTN                 AnchorSize, TablesSize;
  QEMU_SMBIOS_ANCHOR    QemuAnchor;

  if (RETURN_ERROR (QemuFwCfgFindFile ("etc/smbios/smbios-anchor", &Anchor, &AnchorSize)) ||
      RETURN_ERROR (QemuFwCfgFindFile ("etc/smbios/smbios-tables", &Tables, &TablesSize)) ||
      (TablesSize == 0))
  {
      ASSERT (FALSE);
  }

  QemuFwCfgSelectItem (Anchor);

  switch (AnchorSize) {
    case sizeof QemuAnchor.V2:
      QemuFwCfgReadBytes (AnchorSize, &QemuAnchor);

      DEBUG ((DEBUG_INFO, "QemuAnchor.V2.MajorVersion : %d\n", QemuAnchor.V2.MajorVersion));
      DEBUG ((DEBUG_INFO, "QemuAnchor.V2.MinorVersion : %d\n", QemuAnchor.V2.MinorVersion));
      DEBUG ((DEBUG_INFO, "QemuAnchor.V2.TableLength : %d\n", QemuAnchor.V2.TableLength));
      PcdSet16S (PcdSmbiosVersion, (UINT16)(QemuAnchor.V2.MajorVersion << 8 | QemuAnchor.V2.MinorVersion));

      break;

    case sizeof QemuAnchor.V3:
      QemuFwCfgReadBytes (AnchorSize, &QemuAnchor);

      DEBUG ((DEBUG_INFO, "QemuAnchor.V3.MajorVersion : %d\n", QemuAnchor.V3.MajorVersion));
      DEBUG ((DEBUG_INFO, "QemuAnchor.V3.MinorVersion : %d\n", QemuAnchor.V3.MinorVersion));
      DEBUG ((DEBUG_INFO, "QemuAnchor.V3.TableMaximumSize : %d\n", QemuAnchor.V3.TableMaximumSize));
      PcdSet16S (PcdSmbiosVersion, (UINT16)(QemuAnchor.V3.MajorVersion << 8 | QemuAnchor.V3.MinorVersion));
      PcdSet8S (PcdSmbiosDocRev, QemuAnchor.V3.DocRev);

      break;

    default:
      ASSERT (FALSE);
  }

  PcdSetBoolS (PcdQemuSmbiosValidated, TRUE);
  return RETURN_SUCCESS;
}
