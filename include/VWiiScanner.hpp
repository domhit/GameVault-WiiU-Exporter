#pragma once
#include <cstdint>
#include <string>
#include <vector>

struct IOSUHAXProbeRow {
    std::string path;
    int32_t openStatus = 0;
    bool opened = false;
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
    std::vector<IOSUHAXProbeRow> probes;
};
VWiiScanResult ScanVWiiTitles();
bool ExportVWiiDiagnostics(const VWiiScanResult& result, std::string& error);
