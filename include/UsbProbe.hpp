#pragma once
#include <cstdint>
#include <string>
#include <vector>

struct UsbRawProbe {
    std::string devicePath;
    int32_t rawOpenStatus = 0;
    int32_t rawHandle = -1;
    int32_t readStatus = 0;
    bool readable = false;
    std::string signature;
    bool mbrSignature = false;
    bool fatLike = false;
    bool wbfsLike = false;
    std::string sector0Hex;
};

struct UsbProbeResult {
    std::vector<UsbRawProbe> devices;
};

UsbProbeResult ProbeUsbDevices();
bool ExportUsbRawDiagnostics(const UsbProbeResult& result, std::string& error);
