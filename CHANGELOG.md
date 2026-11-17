# Changelog

## v0.2.0

### Added

- Read-only vWii SLCCMPT inventory via IOSUHAX/FSA.
- Wii TMD decoding and vWii title classification.
- USB Loader GX `WII.cache` parser for Wii USB inventory.
- Optional USB Loader GX PlayCount source from `GXGameStatistics.cfg`.
- `wii_usbloadergx.json` and `vwii_titles.json` exports.
- GPL-3.0-or-later licensing and explicit third-party attribution.
- On-screen reminder to refresh USB Loader GX before exporting after Wii-library changes.

### Changed

- Wii U metadata paths now use the exact MCP path, so dynamic `storage_usb01` / `storage_usb02` mounts are handled correctly.
- Exporter version and JSON schema version are maintained independently.

### Removed

- Experimental raw USB, MBR, UStealth and direct `/dev/usbXX` diagnostics from the stable output path.

## v0.1.0

- Initial stable Wii U inventory exporter.
- GAME, UPDATE and DLC enumeration via MCP.
- Rich installed Wii U metadata and atomic JSON export.
- ProcUI-aware console UI and safe return to Wii U Menu.
