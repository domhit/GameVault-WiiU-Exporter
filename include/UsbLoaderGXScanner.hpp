// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
#include <string>
#include <vector>
struct WiiCachedGame { std::string gameId, cacheTitle, path, format; uint32_t playCount=0; };
struct UsbLoaderGXResult { bool cacheFound=false; uint32_t cacheVersion=0; std::string cachePath, statisticsPath, error; std::vector<WiiCachedGame> games; };
UsbLoaderGXResult ScanUsbLoaderGX();
bool ExportUsbLoaderGX(const UsbLoaderGXResult& result,std::string& error);
