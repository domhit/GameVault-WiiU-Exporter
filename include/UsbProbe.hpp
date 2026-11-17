#pragma once
#include <cstdint>
#include <string>
#include <vector>

struct PartitionProbe {
    uint8_t index = 0;
    uint8_t bootFlag = 0;
    uint8_t type = 0;
    uint32_t startLba = 0;
    uint32_t sectorCount = 0;
    uint64_t sizeBytes512 = 0;
    int32_t readStatus = 0;
    bool readable = false;
    bool fatLike = false;
    bool ntfsLike = false;
    bool exfatLike = false;
    bool wbfsLike = false;
    bool bootSignature55AA = false;
    std::string signature;
    std::string sectorHex;
};

struct UsbRawProbe {
    std::string devicePath;
    int32_t rawOpenStatus = 0;
    int32_t rawHandle = -1;
    int32_t readStatus = 0;
    bool readable = false;
    bool allZeroSector0 = false;
    bool standardMbr = false;
    bool uStealthCandidate = false;
    std::string mbrSignature;
    std::string signature;
    std::string sector0Hex;
    std::vector<PartitionProbe> partitions;
};
struct UsbProbeResult { std::vector<UsbRawProbe> devices; };
UsbProbeResult ProbeUsbDevices();
bool ExportUsbRawDiagnostics(const UsbProbeResult& result, std::string& error);
