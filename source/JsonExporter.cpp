#include "JsonExporter.hpp"
#include <cstdio>
#include <cerrno>
#include <cstring>
#include <ctime>
#include <inttypes.h>

namespace {
constexpr const char* FinalPath = "/vol/external01/wiiu/apps/GameVault-WiiU-Exporter/installed_titles.json";
constexpr const char* TempPath = "/vol/external01/wiiu/apps/GameVault-WiiU-Exporter/installed_titles.json.tmp";
void String(FILE* f, const std::string& s) {
    std::fputc('"', f);
    for (unsigned char c : s) switch (c) {
        case '"': std::fputs("\\\"", f); break; case '\\': std::fputs("\\\\", f); break;
        case '\n': std::fputs("\\n", f); break; case '\r': std::fputs("\\r", f); break;
        case '\t': std::fputs("\\t", f); break;
        default: if (c < 0x20) std::fprintf(f, "\\u%04X", c); else std::fputc(c, f);
    }
    std::fputc('"', f);
}
void Prop(FILE* f, const char* key, const std::string& value, bool comma=true) {
    std::fprintf(f, "      \"%s\": ", key); String(f, value); std::fputs(comma ? ",\n" : "\n", f);
}
}

bool ExportJson(const std::vector<InstalledTitle>& titles, std::string& error) {
    error.clear(); errno = 0;
    FILE* f = std::fopen(TempPath, "wb");
    if (!f) { error = std::string("fopen: ") + std::strerror(errno); return false; }
    char timestamp[32] = {};
    const std::time_t now = std::time(nullptr);
    if (const std::tm* t = std::localtime(&now)) std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%S%z", t);
    std::fputs("{\n  \"schemaVersion\": 1,\n  \"exporter\": \"GameVault Wii U Exporter\",\n  \"exporterVersion\": \"0.1.0-dev1\",\n  \"platform\": \"WII_U\",\n  \"generatedAt\": ", f);
    String(f, timestamp);
    std::fprintf(f, ",\n  \"titleCount\": %u,\n  \"titles\": [\n", static_cast<unsigned>(titles.size()));
    for (size_t i=0; i<titles.size(); ++i) {
        const auto& t=titles[i];
        std::fprintf(f, "    {\n      \"titleId\": \"%016" PRIX64 "\",\n", t.titleId);
        Prop(f,"name",t.name); Prop(f,"productCode",t.productCode); Prop(f,"kind",t.kind); Prop(f,"region",t.region);
        std::fprintf(f,"      \"installedVersion\": %u,\n",t.version); Prop(f,"storage",t.storage); Prop(f,"path",t.path); Prop(f,"iconPath",t.iconPath);
        std::fprintf(f,"      \"groupId\": %u,\n      \"appType\": %u,\n      \"osVersion\": %" PRIu64 ",\n      \"sdkVersion\": %u,\n",t.groupId,t.appType,t.osVersion,t.sdkVersion);
        Prop(f,"indexedDevice",t.indexedDevice,false); std::fputs(i+1<titles.size()?"    },\n":"    }\n",f);
    }
    std::fputs("  ]\n}\n",f);
    if (std::fflush(f)!=0 || std::ferror(f)) { error=std::string("write: ")+std::strerror(errno); std::fclose(f); std::remove(TempPath); return false; }
    if (std::fclose(f)!=0) { error=std::string("close: ")+std::strerror(errno); std::remove(TempPath); return false; }
    std::remove(FinalPath);
    if (std::rename(TempPath,FinalPath)!=0) { error=std::string("rename: ")+std::strerror(errno); std::remove(TempPath); return false; }
    return true;
}
