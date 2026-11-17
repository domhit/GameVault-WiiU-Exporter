# Third-party software and licenses

GameVault Wii U Exporter uses or interoperates with the following projects.
They are not relicensed by this repository; their own licenses continue to
apply.

## Compat Title Installer

- Project: `Xpl0itU/compat-installer`
- URL: https://github.com/Xpl0itU/compat-installer
- License: GPL-2.0-or-later
- Usage: the vWii read-only SLCCMPT access follows the established
  IOSUHAX/FSA access model used by Compat Title Installer. No channel-install,
  ticket-write, TMD-write, IOS-write, deletion, or NAND modification code is
  used by GameVault Wii U Exporter.

## USB Loader GX

- Project: `wiidev/usbloadergx`
- URL: https://github.com/wiidev/usbloadergx
- License: project source contains GPL-licensed components; relevant cache
  implementation is treated as GPL-compatible for this GPL-3.0-or-later
  release.
- Usage: GameVault Wii U Exporter interoperates with an existing USB Loader GX
  installation by reading `cache/WII.cache` and optionally
  `GXGameStatistics.cfg` from the SD card. USB Loader GX itself is not bundled.

## libmocha

- Project: `wiiu-env/libmocha`
- URL: https://github.com/wiiu-env/libmocha
- License: LGPL-3.0
- Usage: linked external library for Wii U / Mocha filesystem access.

## libiosuhax

- Project: `dimok789/libiosuhax` / maintained Wii U build package
- URL: https://github.com/dimok789/libiosuhax
- License: permissive zlib-style license contained in the upstream source.
- Usage: linked external library for read-only IOSUHAX/FSA vWii access.

## WUT

- Project: `devkitPro/wut`
- URL: https://github.com/devkitPro/wut
- License: zlib
- Usage: Wii U toolchain/runtime APIs including WUHB, ProcUI and console output.

## wuphax

- Project: `FIX94/wuphax`
- URL: https://github.com/FIX94/wuphax
- License: MIT
- Usage: historical reference for Wii U-mode access to vWii SLCCMPT. No wuphax
  binary is bundled.
