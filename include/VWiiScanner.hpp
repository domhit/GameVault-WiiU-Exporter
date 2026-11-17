#pragma once
#include <cstdint>
#include <string>
#include <vector>

struct VWiiTitleDiagnostic {
    std::string titleId;
    std::string upperId;
    std::string lowerId;
    std::string titlePath;
    std::string tmdPath;
    bool hasTmd = false;
    uint64_t tmdSize = 0;
    uint32_t contentFileCount = 0;
    bool hasBanner = false;
    std::string category;
};

struct VWiiScanResult {
    bool available = false;
    int32_t mountStatus = 0;
    std::string mountStatusText;
    std::string rootPath;
    std::string error;
    std::vector<VWiiTitleDiagnostic> titles;
};

VWiiScanResult ScanVWiiTitles();
bool ExportVWiiDiagnostics(const VWiiScanResult& result, std::string& error);
