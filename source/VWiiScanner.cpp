#include "VWiiScanner.hpp"
#include <mocha/mocha.h>
#include <dirent.h>
#include <sys/stat.h>
#include <cstdio>
#include <cerrno>
#include <cstring>
#include <algorithm>
#include <cctype>

namespace {
constexpr const char* MountName = "storage_slccmpt01";
constexpr const char* MountPath = "/vol/storage_slccmpt01";
constexpr const char* TitleRoot = "/vol/storage_slccmpt01/title";
constexpr const char* OutputPath = "/vol/external01/wiiu/apps/GameVault-WiiU-Exporter/vwii_diagnostics.json";
constexpr const char* TempPath = "/vol/external01/wiiu/apps/GameVault-WiiU-Exporter/vwii_diagnostics.json.tmp";

bool IsHex8(const char* value) {
    if (!value || std::strlen(value) != 8) return false;
    for (const char* p = value; *p; ++p) if (!((*p >= '0' && *p <= '9') || (*p >= 'a' && *p <= 'f') || (*p >= 'A' && *p <= 'F'))) return false;
    return true;
}
std::string Upper(std::string value) { std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c){ return static_cast<char>(std::toupper(c)); }); return value; }
bool RegularFile(const std::string& path, uint64_t* size = nullptr) { struct stat st{}; if (stat(path.c_str(), &st) != 0 || !S_ISREG(st.st_mode)) return false; if (size) *size = static_cast<uint64_t>(st.st_size); return true; }
uint32_t CountContentFiles(const std::string& path) { DIR* dir = opendir(path.c_str()); if (!dir) return 0; uint32_t count = 0; while (dirent* entry = readdir(dir)) { if (entry->d_name[0] == '.') continue; std::string name = entry->d_name; if (name.size() >= 4 && name.substr(name.size()-4) == ".app") ++count; } closedir(dir); return count; }
std::string Category(const std::string& upper) {
    if (upper == "00000001") return "SYSTEM";
    if (upper == "00010001") return "CHANNEL_OR_WIIWARE";
    if (upper == "00010002") return "SYSTEM_CHANNEL";
    if (upper == "00010004") return "GAME_CHANNEL";
    if (upper == "00010005") return "DLC";
    if (upper == "00010008") return "HIDDEN_CHANNEL";
    return "OTHER";
}
void JsonString(FILE* f, const std::string& value) { fputc('"', f); for (unsigned char c : value) { switch(c) { case '"': fputs("\\\"",f); break; case '\\': fputs("\\\\",f); break; case '\n': fputs("\\n",f); break; case '\r': fputs("\\r",f); break; case '\t': fputs("\\t",f); break; default: if(c<0x20) fprintf(f,"\\u%04X",c); else fputc(c,f); } } fputc('"',f); }
void Property(FILE* f,const char* key,const std::string& value,bool comma=true){fprintf(f,"      \"%s\": ",key);JsonString(f,value);fputs(comma?",\n":"\n",f);}
}

VWiiScanResult ScanVWiiTitles() {
    VWiiScanResult result; result.rootPath = TitleRoot;
    const MochaUtilsStatus status = Mocha_MountFS(MountName, nullptr, MountPath);
    result.mountStatus = static_cast<int32_t>(status); result.mountStatusText = Mocha_GetStatusStr(status);
    if (status != MOCHA_RESULT_SUCCESS && status != MOCHA_RESULT_ALREADY_EXISTS) { result.error = "SLCCMPT mount failed"; return result; }
    DIR* upperDir = opendir(TitleRoot);
    if (!upperDir) { result.error = std::string("opendir title root failed: ") + std::strerror(errno); return result; }
    result.available = true;
    while (dirent* upperEntry = readdir(upperDir)) {
        if (!IsHex8(upperEntry->d_name)) continue;
        const std::string upper = Upper(upperEntry->d_name); const std::string upperPath = std::string(TitleRoot) + "/" + upperEntry->d_name;
        DIR* lowerDir = opendir(upperPath.c_str()); if (!lowerDir) continue;
        while (dirent* lowerEntry = readdir(lowerDir)) {
            if (!IsHex8(lowerEntry->d_name)) continue;
            VWiiTitleDiagnostic row; row.upperId = upper; row.lowerId = Upper(lowerEntry->d_name); row.titleId = row.upperId + row.lowerId;
            row.titlePath = upperPath + "/" + lowerEntry->d_name; row.tmdPath = row.titlePath + "/content/title.tmd"; row.hasTmd = RegularFile(row.tmdPath,&row.tmdSize);
            row.contentFileCount = CountContentFiles(row.titlePath + "/content");
            row.hasBanner = RegularFile(row.titlePath + "/data/banner.bin") || RegularFile(row.titlePath + "/content/00000000.app");
            row.category = Category(row.upperId); result.titles.push_back(std::move(row));
        }
        closedir(lowerDir);
    }
    closedir(upperDir);
    std::sort(result.titles.begin(), result.titles.end(), [](const auto& a,const auto& b){return a.titleId < b.titleId;});
    return result;
}

bool ExportVWiiDiagnostics(const VWiiScanResult& r, std::string& error) {
    error.clear(); FILE* f = fopen(TempPath,"wb"); if(!f){error=std::string("fopen: ")+std::strerror(errno);return false;}
    fputs("{\n  \"schemaVersion\": 1,\n  \"exporterVersion\": \"0.2.0-dev1\",\n  \"diagnostic\": \"VWII_SLCCMPT\",\n",f);
    fprintf(f,"  \"available\": %s,\n  \"mountStatus\": %d,\n  \"mountStatusText\": ",r.available?"true":"false",r.mountStatus);JsonString(f,r.mountStatusText);
    fputs(",\n  \"rootPath\": ",f);JsonString(f,r.rootPath);fputs(",\n  \"error\": ",f);JsonString(f,r.error);fprintf(f,",\n  \"titleCount\": %u,\n  \"titles\": [\n",static_cast<unsigned>(r.titles.size()));
    for(size_t i=0;i<r.titles.size();++i){const auto&t=r.titles[i];fputs("    {\n",f);Property(f,"titleId",t.titleId);Property(f,"upperId",t.upperId);Property(f,"lowerId",t.lowerId);Property(f,"category",t.category);Property(f,"titlePath",t.titlePath);Property(f,"tmdPath",t.tmdPath);fprintf(f,"      \"hasTmd\": %s,\n      \"tmdSize\": %llu,\n      \"contentFileCount\": %u,\n      \"hasBanner\": %s\n",t.hasTmd?"true":"false",static_cast<unsigned long long>(t.tmdSize),t.contentFileCount,t.hasBanner?"true":"false");fputs(i+1<r.titles.size()?"    },\n":"    }\n",f);}
    fputs("  ]\n}\n",f); if(fflush(f)||ferror(f)){error=std::string("write: ")+std::strerror(errno);fclose(f);remove(TempPath);return false;} if(fclose(f)){error=std::string("close: ")+std::strerror(errno);remove(TempPath);return false;} remove(OutputPath); if(rename(TempPath,OutputPath)){error=std::string("rename: ")+std::strerror(errno);remove(TempPath);return false;} return true;
}
