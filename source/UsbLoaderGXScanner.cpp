// SPDX-License-Identifier: GPL-3.0-or-later
#include "UsbLoaderGXScanner.hpp"
#include <cstdio>
#include <cerrno>
#include <cstring>
#include <map>
#include <algorithm>
#include <cctype>
namespace {
constexpr const char* Base="/vol/external01/apps/usbloader_gx";
constexpr const char* Cache="/vol/external01/apps/usbloader_gx/cache/WII.cache";
constexpr const char* Stats="/vol/external01/apps/usbloader_gx/GXGameStatistics.cfg";
constexpr const char* Out="/vol/external01/wiiu/apps/GameVault-WiiU-Exporter/wii_usbloadergx.json";
constexpr const char* Tmp="/vol/external01/wiiu/apps/GameVault-WiiU-Exporter/wii_usbloadergx.json.tmp";
std::string Field(const unsigned char*p,size_t n){size_t l=0;while(l<n&&p[l])++l;return std::string(reinterpret_cast<const char*>(p),l);}
void JsonString(FILE*f,const std::string&v){fputc('"',f);for(unsigned char c:v){switch(c){case'"':fputs("\\\"",f);break;case'\\':fputs("\\\\",f);break;case'\n':fputs("\\n",f);break;case'\r':fputs("\\r",f);break;case'\t':fputs("\\t",f);break;default:if(c<0x20)fprintf(f,"\\u%04X",c);else fputc(c,f);}}fputc('"',f);}
bool ValidId(const std::string&s){if(s.size()!=6)return false;for(char c:s)if(!std::isalnum(static_cast<unsigned char>(c)))return false;return true;}
std::map<std::string,uint32_t> ReadStats(){std::map<std::string,uint32_t> out;FILE*f=fopen(Stats,"rb");if(!f)return out;char line[512];while(fgets(line,sizeof(line),f)){std::string s=line;auto eq=s.find('=');if(eq==std::string::npos)continue;std::string id=s.substr(0,eq);if(!ValidId(id))continue;auto pc=s.find("PlayCount:",eq);if(pc!=std::string::npos)out[id]=static_cast<uint32_t>(std::strtoul(s.c_str()+pc+10,nullptr,10));}fclose(f);return out;}
}
UsbLoaderGXResult ScanUsbLoaderGX(){UsbLoaderGXResult r;r.cachePath=Cache;r.statisticsPath=Stats;FILE*f=fopen(Cache,"rb");if(!f){r.error=std::string("WII.cache: ")+strerror(errno);return r;}fseek(f,0,SEEK_END);long size=ftell(f);rewind(f);if(size<4||(size-4)%364){r.error="Unexpected WII.cache record size";fclose(f);return r;}unsigned char h[4];if(fread(h,1,4,f)!=4){r.error="WII.cache header read failed";fclose(f);return r;}r.cacheVersion=(uint32_t(h[0])<<24)|(uint32_t(h[1])<<16)|(uint32_t(h[2])<<8)|h[3];auto stats=ReadStats();const long count=(size-4)/364;for(long i=0;i<count;++i){unsigned char rec[364];if(fread(rec,1,sizeof(rec),f)!=sizeof(rec))break;WiiCachedGame g;g.gameId=Field(rec,6);g.cacheTitle=Field(rec+32,64);g.path=Field(rec+98,256);if(!ValidId(g.gameId)||g.path.empty())continue;std::string lower=g.path;std::transform(lower.begin(),lower.end(),lower.begin(),[](unsigned char c){return char(std::tolower(c));});g.format=lower.size()>=5&&lower.substr(lower.size()-5)==".wbfs"?"WBFS":(lower.size()>=4&&lower.substr(lower.size()-4)==".iso"?"ISO":"UNKNOWN");auto it=stats.find(g.gameId);if(it!=stats.end())g.playCount=it->second;r.games.push_back(std::move(g));}fclose(f);r.cacheFound=true;return r;}
bool ExportUsbLoaderGX(const UsbLoaderGXResult&r,std::string&error){error.clear();FILE*f=fopen(Tmp,"wb");if(!f){error=strerror(errno);return false;}fputs("{\n  \"schemaVersion\": 1,\n  \"exporterVersion\": \"0.2.0\",\n  \"platform\": \"WII\",\n  \"source\": \"USB Loader GX WII.cache\",\n",f);fprintf(f,"  \"cacheFound\": %s,\n  \"cacheVersion\": %u,\n  \"cachePath\": ",r.cacheFound?"true":"false",r.cacheVersion);JsonString(f,r.cachePath);fputs(",\n  \"statisticsPath\": ",f);JsonString(f,r.statisticsPath);fputs(",\n  \"error\": ",f);JsonString(f,r.error);fprintf(f,",\n  \"gameCount\": %u,\n  \"games\": [\n",unsigned(r.games.size()));for(size_t i=0;i<r.games.size();++i){const auto&g=r.games[i];fputs("    {\"gameId\": ",f);JsonString(f,g.gameId);fputs(", \"title\": ",f);JsonString(f,g.cacheTitle);fputs(", \"path\": ",f);JsonString(f,g.path);fputs(", \"format\": ",f);JsonString(f,g.format);fprintf(f,", \"playCount\": %u}%s\n",g.playCount,i+1<r.games.size()?",":"");}fputs("  ]\n}\n",f);if(fflush(f)||ferror(f)){error="write failed";fclose(f);remove(Tmp);return false;}if(fclose(f)){error="close failed";remove(Tmp);return false;}remove(Out);if(rename(Tmp,Out)){error="rename failed";remove(Tmp);return false;}return true;}
