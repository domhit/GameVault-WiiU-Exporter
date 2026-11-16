# GameVault Wii U Exporter
Independent minimal Wii U inventory exporter for GameVault using public WUT/MCP/libmocha interfaces.

## v0.1.0-dev3
- Enumerates installed Wii U base games, updates and DLC through MCP.
- Reads metadata from the verified Mocha volume paths `/vol/storage_usb01/...` and `/vol/storage_mlc01/...`.
- Writes JSON atomically to `/vol/external01/wiiu/apps/GameVault-WiiU-Exporter/installed_titles.json`.
- Removes explicit `Mocha_UnmountFS` calls on exit while retaining `Mocha_DeInitLibrary`.
- Displays `Returning to Aroma...` before shutdown.

FTP: `/fs/vol/external01/wiiu/apps/GameVault-WiiU-Exporter/installed_titles.json`


### dev3 diagnostics
- Shows `Mocha_InitLibrary` and USB/MLC `Mocha_MountFS` status codes with `Mocha_GetStatusStr`.
- Tests both documented devoptab paths (`storage_usb01:/`, `storage_mlc01:/`) and `/vol/...` fallbacks, showing path plus `errno`.
- Keeps JSON export enabled.
- Diagnostic exit skips `Mocha_UnmountFS`, `Mocha_DeInitLibrary`, and `VPADShutdown`; it frees the app-owned screen buffers and calls only `WHBProcShutdown`.
