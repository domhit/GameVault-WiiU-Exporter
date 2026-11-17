#pragma once
#include <cstdint>
#include <string>
struct InstalledTitle {
    uint64_t titleId = 0; uint16_t version = 0; std::string name; std::string productCode;
    std::string kind; std::string region; std::string storage; std::string path; std::string iconPath;
    std::string publisher; std::string companyCode; std::string contentPlatform; uint32_t xmlTitleVersion = 0; uint32_t drcUse = 0; uint32_t networkUse = 0; uint32_t onlineAccountUse = 0;
    uint32_t groupId = 0; uint32_t appType = 0; uint64_t osVersion = 0; uint32_t sdkVersion = 0; std::string indexedDevice;
};
