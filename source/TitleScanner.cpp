#include "TitleScanner.hpp"
#include <coreinit/mcp.h>
#include <cstdio>
#include <cstring>
#include <inttypes.h>

namespace {
std::string Trim(std::string value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

std::string DecodeEntities(std::string value) {
    struct Pair { const char* from; const char* to; } pairs[] = {
        {"&amp;", "&"}, {"&apos;", "'"}, {"&quot;", "\""}, {"&lt;", "<"}, {"&gt;", ">"}
    };
    for (const auto& pair : pairs) {
        size_t pos = 0;
        while ((pos = value.find(pair.from, pos)) != std::string::npos) {
            value.replace(pos, std::strlen(pair.from), pair.to);
            pos += std::strlen(pair.to);
        }
    }
    return value;
}

std::string ReadXmlTag(const std::string& path, const char* tag) {
    FILE* file = std::fopen(path.c_str(), "rb");
    if (!file) return {};
    std::string data;
    char buffer[2048];
    while (const size_t count = std::fread(buffer, 1, sizeof(buffer), file)) data.append(buffer, count);
    std::fclose(file);
    const std::string open = std::string("<") + tag + ">";
    const std::string close = std::string("</") + tag + ">";
    const auto begin = data.find(open);
    if (begin == std::string::npos) return {};
    const auto valueBegin = begin + open.size();
    const auto end = data.find(close, valueBegin);
    if (end == std::string::npos) return {};
    return DecodeEntities(Trim(data.substr(valueBegin, end - valueBegin)));
}

std::string FilesystemBase(const MCPTitleListType& source) {
    const std::string original = source.path;
    const auto usr = original.find("/usr/");
    if (usr == std::string::npos) return {};
    const std::string device = source.indexedDevice;
    if (device == "usb") return "storage_usb01:" + original.substr(usr);
    if (device == "mlc") return "storage_mlc01:" + original.substr(usr);
    return {};
}

std::string Kind(uint64_t id, uint32_t appType) {
    if (appType == static_cast<uint32_t>(MCP_APP_TYPE_GAME_WII)) return "WII";
    switch (static_cast<uint32_t>(id >> 32)) {
        case 0x00050000: return "GAME";
        case 0x0005000C: return "DLC";
        case 0x0005000E: return "UPDATE";
        default: return "SYSTEM_OR_OTHER";
    }
}

std::string Region(const std::string& raw) {
    if (raw.empty()) return "ALL";
    const unsigned value = std::strtoul(raw.c_str(), nullptr, 0);
    switch (value & 7U) {
        case 1: return "JPN"; case 2: return "USA"; case 3: return "JPN/USA";
        case 4: return "EUR"; case 5: return "EUR/JPN"; case 6: return "EUR/USA";
        default: return "ALL";
    }
}
}

bool ScanInstalledTitles(std::vector<InstalledTitle>& titles, std::string& error) {
    titles.clear(); error.clear();
    const int32_t handle = MCP_Open();
    if (handle < 0) { error = "MCP_Open failed"; return false; }
    const MCPError countResult = MCP_TitleCount(handle);
    if (countResult < 0) { MCP_Close(handle); error = "MCP_TitleCount failed"; return false; }
    uint32_t count = static_cast<uint32_t>(countResult);
    std::vector<MCPTitleListType> records(count);
    uint32_t fetched = count;
    if (count && MCP_TitleList(handle, &fetched, records.data(), records.size() * sizeof(MCPTitleListType)) < 0) {
        MCP_Close(handle); error = "MCP_TitleList failed"; return false;
    }
    MCP_Close(handle);
    records.resize(fetched);
    titles.reserve(fetched);
    for (const auto& source : records) {
        const uint32_t prefix = static_cast<uint32_t>(source.titleId >> 32);
        if (prefix != 0x00050000 && prefix != 0x0005000C && prefix != 0x0005000E) continue;
        InstalledTitle title;
        title.titleId = source.titleId;
        title.version = source.titleVersion;
        title.path = source.path;
        title.groupId = source.groupId;
        title.appType = static_cast<uint32_t>(source.appType);
        title.osVersion = source.osVersion;
        title.sdkVersion = source.sdkVersion;
        title.indexedDevice = source.indexedDevice;
        title.storage = title.indexedDevice == "usb" ? "USB" : (title.indexedDevice == "mlc" ? "NAND" : "UNKNOWN");
        title.kind = Kind(title.titleId, title.appType);
        const std::string base = FilesystemBase(source);
        const std::string meta = base + "/meta/meta.xml";
        title.name = ReadXmlTag(meta, "longname_en");
        if (title.name.empty()) title.name = ReadXmlTag(meta, "shortname_en");
        if (title.name.empty()) { char id[17]; std::snprintf(id, sizeof(id), "%016" PRIX64, title.titleId); title.name = id; }
        title.productCode = ReadXmlTag(meta, "product_code");
        title.region = Region(ReadXmlTag(meta, "region"));
        title.iconPath = base + "/meta/iconTex.tga";
        titles.push_back(std::move(title));
    }
    return true;
}
