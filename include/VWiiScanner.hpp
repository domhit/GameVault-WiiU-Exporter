// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
#include <string>
#include <vector>

struct VWiiTitle {
    std::string titleId, upperId, lowerId, asciiId, category, titlePath, tmdPath;
    int32_t tmdOpenStatus = 0;
    uint64_t tmdSize = 0, systemVersion = 0, tmdTitleId = 0;
    uint32_t titleType = 0, accessRights = 0, contentCount = 0;
    uint16_t groupId = 0, regionCode = 0, titleVersion = 0, tmdContentCount = 0, bootIndex = 0;
    bool tmdDecoded = false;
};
struct VWiiScanResult {
    bool available = false;
    std::string backend = "IOSUHAX_FSA";
    int32_t iosuhaxMcpStatus = 0, iosuhaxDefaultStatus = 0, fsaHandle = -1, mountStatus = 0;
    std::string devicePath = "/dev/slccmpt01", volumePath = "/vol/storage_slccmpt01", error;
    std::vector<VWiiTitle> titles;
};
VWiiScanResult ScanVWiiTitles();
bool ExportVWiiDiagnostics(const VWiiScanResult& result, std::string& error);
