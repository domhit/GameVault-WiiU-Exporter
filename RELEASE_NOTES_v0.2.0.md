# GameVault Wii U Exporter v0.2.0

v0.2.0 expands the exporter from Wii U inventory to a combined Wii U + vWii/Wii data source for GameVault.

## Highlights

- Wii U GAME / UPDATE / DLC inventory remains fully supported.
- vWii NAND titles are enumerated read-only and their TMD metadata is decoded.
- Wii games are imported from USB Loader GX's current `WII.cache` on SD, avoiding direct access to UStealth-hidden USB storage.
- Existing USB Loader GX PlayCount data can be included when `GXGameStatistics.cfg` is present.

## Important Wii cache note

After changing the Wii games on the vWii USB drive, launch USB Loader GX once and let its game list load before running the exporter. Otherwise `wii_usbloadergx.json` may reflect the previous cache state.

## Files

- `installed_titles.json` — Wii U
- `vwii_titles.json` — vWii NAND
- `wii_usbloadergx.json` — Wii games known to USB Loader GX

## Licensing

v0.2.0 is GPL-3.0-or-later. See `LICENSE`, `NOTICE`, and `THIRD_PARTY_LICENSES.md`.
