# @cecekpawon 2021

[Defines]
  PLATFORM_NAME           = UefiSevenPkg
  PLATFORM_GUID           = 86972B2A-2A21-492D-88CF-2906C3E273AF
  PLATFORM_VERSION        = 1.0
  SUPPORTED_ARCHITECTURES = X64
  BUILD_TARGETS           = RELEASE|DEBUG
  SKUID_IDENTIFIER        = DEFAULT
  DSC_SPECIFICATION       = 0x00010006

[LibraryClasses]
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
!if $(TARGET) == DEBUG
  DebugLib|MdePkg/Library/UefiDebugLibConOut/UefiDebugLibConOut.inf
  DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
!else
  DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
!endif

  CpuLib|MdePkg/Library/BaseCpuLib/BaseCpuLib.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  IoLib|MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  MtrrLib|UefiCpuPkg/Library/MtrrLib/MtrrLib.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf

[Components]
  UefiSevenPkg/Platform/UefiSeven/UefiSeven.inf

[BuildOptions]
  DEFINE UEFISEVENPKG_BUILD_OPTIONS_GEN = -D DISABLE_NEW_DEPRECATED_INTERFACES $(UEFISEVENPKG_BUILD_OPTIONS) -D TARGET_BUILD_$(TARGET)

  GCC:DEBUG_*_*_CC_FLAGS     = $(UEFISEVENPKG_BUILD_OPTIONS_GEN)
  GCC:RELEASE_*_*_CC_FLAGS   = -D MDEPKG_NDEBUG $(UEFISEVENPKG_BUILD_OPTIONS_GEN)
  MSFT:DEBUG_*_*_CC_FLAGS    = $(UEFISEVENPKG_BUILD_OPTIONS_GEN)
  MSFT:RELEASE_*_*_CC_FLAGS  = /D MDEPKG_NDEBUG $(UEFISEVENPKG_BUILD_OPTIONS_GEN)
  XCODE:RELEASE_*_*_CC_FLAGS = -Wno-varargs -flto -D MDEPKG_NDEBUG $(UEFISEVENPKG_BUILD_OPTIONS_GEN)
  XCODE:DEBUG_*_*_CC_FLAGS   = -Wno-varargs $(UEFISEVENPKG_BUILD_OPTIONS_GEN)
  RELEASE_*_*_GENFW_FLAGS    = --zero
