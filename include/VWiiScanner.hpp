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
struct VWiiUsbProbe {
    std::string devicePath, volumePath, rootPath, wbfsPath;
    int32_t mountStatus = 0, rootOpenStatus = 0, wbfsOpenStatus = 0;
    bool rootOpened = false, wbfsOpened = false;
    std::vector<std::string> rootEntries;
    std::vector<std::string> wbfsEntries;
};
struct VWiiScanResult {
    bool available = false;
    std::string backend = "IOSUHAX_FSA";
    int32_t iosuhaxMcpStatus = 0, iosuhaxDefaultStatus = 0, fsaHandle = -1, mountStatus = 0;
    std::string devicePath = "/dev/slccmpt01", volumePath = "/vol/storage_slccmpt01", error;
    std::vector<VWiiTitle> titles;
    std::vector<VWiiUsbProbe> usbProbes;
};
VWiiScanResult ScanVWiiTitles();
bool ExportVWiiDiagnostics(const VWiiScanResult& result, std::string& error);
