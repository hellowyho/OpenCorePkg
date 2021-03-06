/** @file
  Copyright (C) 2019, vit9696. All rights reserved.

  All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#include "FwRuntimeServicesPrivate.h"

#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/OcBootManagementLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Protocol/LoadedImage.h>
#include <Protocol/OcFirmwareRuntime.h>

STATIC
VOID
EFIAPI
FwGetCurrent (
  OUT OC_FWRT_CONFIG  *Config
  )
{
  CopyMem (Config, gCurrentConfig, sizeof (*Config));
}

STATIC
VOID
EFIAPI
FwSetMain (
  IN CONST OC_FWRT_CONFIG  *Config
  )
{
  CopyMem (&gMainConfig, Config, sizeof (gMainConfig));
}

STATIC
VOID
EFIAPI
FwSetOverride (
  IN CONST OC_FWRT_CONFIG  *Config
  )
{
  if (Config != NULL) {
    CopyMem (&gOverrideConfig, Config, sizeof (gOverrideConfig));
    gCurrentConfig = &gOverrideConfig;
  } else {
    gCurrentConfig = &gMainConfig;
  }
}

STATIC
OC_FIRMWARE_RUNTIME_PROTOCOL
mOcFirmwareRuntimeProtocol = {
  OC_FIRMWARE_RUNTIME_REVISION,
  FwGetCurrent,
  FwSetMain,
  FwSetOverride,
  FwOnGetVariable
};

EFI_STATUS
EFIAPI
UefiEntrypoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS   Status;
  VOID         *Interface;
  EFI_HANDLE   Handle;

  Status = gBS->LocateProtocol (
    &gOcFirmwareRuntimeProtocolGuid,
    NULL,
    &Interface
    );

  if (!EFI_ERROR (Status)) {
    //
    // In case for whatever reason one tried to reload the driver.
    //
    return EFI_ALREADY_STARTED;
  }

  //
  // Activate main configuration.
  //
  gCurrentConfig = &gMainConfig;

  RedirectRuntimeServices ();

  Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
    &Handle,
    &gOcFirmwareRuntimeProtocolGuid,
    &mOcFirmwareRuntimeProtocol,
    NULL
    );

  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}
