# GameVault Wii U Exporter
Independent minimal Wii U inventory exporter for GameVault using public WUT/MCP/libmocha interfaces.

## v0.1.0-dev6
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
