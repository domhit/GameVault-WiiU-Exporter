#pragma once
#include <cstdint>
#include <string>
#include <vector>

struct FSAProbeRow {
    std::string path;
    int32_t openStatus = 0;
    std::string statusText;
    bool opened = false;
};
struct VWiiScanResult {
    bool available = false;
    int32_t mountStatus = 0;
    std::string mountStatusText;
    int32_t fsaInitStatus = 0;
    int32_t clientHandle = -1;
    std::string error;
    std::vector<FSAProbeRow> probes;
};
VWiiScanResult ScanVWiiTitles();
bool ExportVWiiDiagnostics(const VWiiScanResult& result, std::string& error);
