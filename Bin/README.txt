# UefiSeven

## Usage instructions

Chainloading bootx64.efi :

Navigate to \EFI\Boot\
Move bootx64.efi -> bootx64.original.efi
Move UefiSeven.efi -> bootx64.efi

Chainloading bootmgfw.efi :

Navigate to \EFI\Microsoft\Boot\
Move bootmgfw.efi -> bootmgfw.original.efi
Move UefiSeven.efi -> bootmgfw.efi

## Settings

UefiSeven will try to read UefiSeven.ini as main setting from now, and fallback to get old files bellow if it doesn't exists.

Currently settings can be applied by creating empty files of specific name in the directory containing the target efi file.

* UefiSeven.verbose : Enables verbose mode
* UefiSeven.skiperrors : Skips any warning/error prompts
* UefiSeven.force_fakevesa : Overwrites Int10h handler with fakevesa even when the native handler is present
