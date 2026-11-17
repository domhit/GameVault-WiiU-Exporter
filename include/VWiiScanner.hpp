#pragma once
#include <cstdint>
#include <string>
#include <vector>

struct VWiiTitle {
    std::string titleId;
    std::string upperId;
    std::string lowerId;
    std::string category;
    std::string titlePath;
    std::string tmdPath;
    int32_t tmdOpenStatus = 0;
    uint64_t tmdSize = 0;
    uint32_t contentCount = 0;
};
struct VWiiScanResult {
    bool available = false;
    std::string backend = "IOSUHAX_FSA";
    int32_t iosuhaxMcpStatus = 0;
    int32_t iosuhaxDefaultStatus = 0;
    int32_t fsaHandle = -1;
    int32_t mountStatus = 0;
    std::string devicePath = "/dev/slccmpt01";
    std::string volumePath = "/vol/storage_slccmpt01";
    std::string error;
    std::vector<VWiiTitle> titles;
};
VWiiScanResult ScanVWiiTitles();
bool ExportVWiiDiagnostics(const VWiiScanResult& result, std::string& error);
