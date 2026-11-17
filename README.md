# GameVault Wii U Exporter
Independent minimal Wii U inventory exporter for GameVault using public WUT/MCP/libmocha interfaces.

## v0.1.0
- Enumerates installed Wii U base games, updates and DLC through MCP.
- Reads metadata from the verified Mocha volume paths `/vol/storage_usb01/...` and `/vol/storage_mlc01/...`.
- Writes JSON atomically to `/vol/external01/wiiu/apps/GameVault-WiiU-Exporter/installed_titles.json`.
- Removes explicit `Mocha_UnmountFS` calls on exit while retaining `Mocha_DeInitLibrary`.
- Displays `Returning to Aroma...` before shutdown.

FTP: `/fs/vol/external01/wiiu/apps/GameVault-WiiU-Exporter/installed_titles.json`


### dev4
10 x 100 ms metadata retry and SYSLaunchMenu lifecycle.


### dev5 WHBLogConsole / ProcUI framebuffer fix
- Replaces all custom `OSScreen` buffers and heap management with WUT `WHBLogConsole`.
- `WHBLogConsole` owns its ProcUI ACQUIRE/RELEASE callbacks, avoiding stale app framebuffers during the transition to the Wii U Menu.
- Keeps dev4 metadata retry (10 x 100 ms) and `SYSLaunchMenu()` for A.
- Mocha unmount/deinit remain disabled for isolation.


### dev6 raw metadata sample
- Keeps the proven dev5 WHBLogConsole / ProcUI exit lifecycle unchanged.
- Writes the full successfully read `meta.xml` from the first GAME record to `/vol/external01/wiiu/apps/GameVault-WiiU-Exporter/meta_sample.xml`.
- Displays sample Title ID, byte size, and whether `longname_en`, `shortname_en`, `product_code`, and `region` were found by the current parser.
- Existing JSON export remains enabled.


### dev7 full metadata parser
- XML tag reader now accepts attributes between tag name and `>`, matching real Wii U `meta.xml`.
- Exports localized English name, product code, publisher, company code, content platform, XML title version, DRC use, network use, and online-account use.
- Keeps region decoding, MCP raw fields, 10 x 100 ms file-open retry, atomic JSON export, and the proven WHBLogConsole/ProcUI lifecycle.
- Raw `meta_sample.xml` generation is disabled now that the format is understood.


## v0.1.0 release
The first stable GameVault Wii U Exporter release uses a black WUT/WHBLogConsole interface, exports rich Wii U metadata, and retains the ProcUI-safe A/HOME return behavior validated on hardware. Pushing tag `v0.1.0` builds the WUHB and publishes it as a GitHub Release asset automatically.


### v0.2.0-dev1 — vWii SLCCMPT diagnostics
- Keeps the stable Wii U `installed_titles.json` schema unchanged.
- Attempts a read-only Mocha mount of `/vol/storage_slccmpt01`.
- Enumerates vWii `/title/<upper>/<lower>` folders and records Title IDs, category, TMD presence/size, `.app` content count and banner candidates.
- Writes `/vol/external01/wiiu/apps/GameVault-WiiU-Exporter/vwii_diagnostics.json`.
- Does not modify, install or delete vWii content.


### v0.2.0-dev3 — read-only FSA probe
Uses WUT's direct FSA API to probe `/vol/storage_slccmpt01`, `/title`, `/ticket`, `/shared1`, and `/sys`. Writes `vwii_fsa_diagnostics.json`; no directory content is modified.

### v0.2.0-dev4 — GPL IOSUHAX/FSA vWii probe

The v0.2 development branch is licensed under GPL-2.0-or-later. The historical
v0.1.0 release remains under the license shipped with that tag.

This build uses the proven IOSUHAX/FSA access model used by Compat Title
Installer and wuphax, reduced to read-only directory probes:

- opens IOSUHAX via `/dev/mcp`, with `/dev/iosuhax` fallback;
- opens an IOSUHAX FSA client;
- mounts `/dev/slccmpt01` globally at `/vol/storage_slccmpt01`;
- probes the root, `title`, `ticket`, `shared1`, and `sys` directories;
- writes `vwii_iosuhax_diagnostics.json` to SD;
- performs no NAND write, install, delete, ticket, TMD, or IOS operation.


### v0.2.0-dev5
- Fixes Wii U metadata by using the exact MCP `source.path` (including dynamic `storage_usb01`/`storage_usb02`).
- Reports the real exporter version while retaining Wii U schema version 1.
- Enumerates vWii title groups and title IDs through read-only IOSUHAX FSA, including title category, TMD presence/size, and content count.
- Writes `vwii_titles.json`.
- USB Loader GX/WBFS media is not part of the vWii NAND title tree and will be handled as a separate removable-storage source.


### v0.2.0-dev6
- Decodes read-only Wii TMD header fields for every vWii title.
- Adds ASCII short IDs and distinguishes disc-title data, channels, system channels, game channels, DLC and hidden channels.
- Probes `/dev/usb01` and `/dev/usb02` through IOSUHAX FSA and lists root plus `/wbfs` entries in `vwii_usb_diagnostics.json`.
- USB probing is diagnostic and read-only; FAT32/WBFS interpretation follows after identifying the real device/filesystem.


### v0.2.0-dev7 — raw vWii USB probe
- Probes candidate USB/UMS raw IOSU devices read-only.
- Reads only sector 0 (512 bytes) from devices that open successfully.
- Detects 0x55AA MBR signature, FAT markers, and WBFS magic.
- Writes `vwii_usb_raw_diagnostics.json`.
- Windows drive-letter/hidden-partition state is irrelevant to this raw IOSU probe.


### v0.2.0-dev8 — UStealth-aware MBR/partition probe
- Reads the complete 512-byte MBR sector of reachable raw USB devices.
- Recognizes standard `55 AA` and UStealth-style `55 AB` signatures.
- Parses all four MBR partition entries (type, start LBA, sector count, estimated size).
- Reads sector 0 of every populated partition and detects FAT, NTFS, exFAT, WBFS and `55 AA` boot signatures.
- Writes `vwii_usb_mbr_diagnostics.json`; all access remains read-only.


### v0.2.0-dev9 — USB Loader GX cache inventory
- Replaces raw USB/UStealth experiments with USB Loader GX's authoritative scanned `cache/WII.cache` on SD.
- Parses the observed fixed format: 4-byte cache version followed by 364-byte game records.
- Exports Game ID, cached title, exact `usb1:/wbfs/...` path, WBFS/ISO format and available PlayCount from `GXGameStatistics.cfg`.
- Writes `wii_usbloadergx.json`.
- Existing Wii U and vWii NAND/TMD exports remain enabled.
