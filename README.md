# GameVault Wii U Exporter

Independent minimal Wii U inventory exporter for GameVault. This project is a clean implementation based on public WUT/MCP/libmocha interfaces and does not include code or assets from Wii-U-TitlesInfo.

## v0.1.0-dev1

- Enumerates installed Wii U base games, updates and DLC through MCP.
- Reads English title name, product code and region from the installed `meta/meta.xml`.
- Preserves separate USB and NAND records.
- Writes JSON atomically to `/vol/external01/wiiu/apps/GameVault-WiiU-Exporter/installed_titles.json`.
- Shows scan/export result on TV and GamePad.
- Press A or HOME to exit.

FTP path: `/fs/vol/external01/wiiu/apps/GameVault-WiiU-Exporter/installed_titles.json`

## Installation

Create `sd:/wiiu/apps/GameVault-WiiU-Exporter/`, copy the WUHB into it and launch it through Aroma. The directory must already exist because the exporter intentionally does not create SD directories.

## Build

Push to GitHub and download the `GameVault-WiiU-Exporter-v0.1.0-dev1` Actions artifact. The workflow uses `devkitpro/devkitppc:latest` plus `ghcr.io/wiiu-env/libmocha:latest` and does not run pacman.

## License

Own project code: MIT. libmocha and WUT are external dependencies governed by their respective licenses.
