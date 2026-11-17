#include "TitleScanner.hpp"
#include <coreinit/mcp.h>
#include <coreinit/thread.h>
#include <coreinit/time.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <inttypes.h>
namespace {
std::string Trim(std::string v){auto a=v.find_first_not_of(" \t\r\n");if(a==std::string::npos)return{};auto b=v.find_last_not_of(" \t\r\n");return v.substr(a,b-a+1);}
std::string DecodeEntities(std::string v){struct P{const char*f;const char*t;}p[]={{"&amp;","&"},{"&apos;","'"},{"&quot;","\""},{"&lt;","<"},{"&gt;",">"}};for(auto&x:p){size_t z=0;while((z=v.find(x.f,z))!=std::string::npos){v.replace(z,std::strlen(x.f),x.t);z+=std::strlen(x.t);}}return v;}
bool ReadRetry(const std::string&p,std::string&d,unsigned&a,int&e){d.clear();for(a=1;a<=10;++a){errno=0;FILE*f=std::fopen(p.c_str(),"rb");if(f){char b[2048];while(size_t n=std::fread(b,1,sizeof(b),f))d.append(b,n);std::fclose(f);return true;}e=errno;if(a<10)OSSleepTicks(OSMillisecondsToTicks(100));}return false;}
std::string Tag(const std::string& d,const char* t){
    const std::string open="<"+std::string(t);
    const std::string close="</"+std::string(t)+">";
    size_t start=d.find(open);
    while(start!=std::string::npos){
        const size_t afterName=start+open.size();
        if(afterName<d.size()&&(d[afterName]=='>'||d[afterName]==' '||d[afterName]=='\t'||d[afterName]=='\r'||d[afterName]=='\n')){
            const size_t valueStart=d.find('>',afterName);
            if(valueStart==std::string::npos)return{};
            const size_t end=d.find(close,valueStart+1);
            if(end==std::string::npos)return{};
            return DecodeEntities(Trim(d.substr(valueStart+1,end-(valueStart+1))));
        }
        start=d.find(open,afterName);
    }
    return{};
}
uint32_t UIntTag(const std::string& d,const char* t){const std::string v=Tag(d,t);return v.empty()?0u:static_cast<uint32_t>(std::strtoul(v.c_str(),nullptr,0));}
std::string Base(const MCPTitleListType&s){std::string p=s.path;auto u=p.find("/usr/");if(u==std::string::npos)return{};std::string d=s.indexedDevice;if(d=="usb")return "/vol/storage_usb01"+p.substr(u);if(d=="mlc")return "/vol/storage_mlc01"+p.substr(u);return{};}
std::string Kind(uint64_t id,uint32_t a){if(a==static_cast<uint32_t>(MCP_APP_TYPE_GAME_WII))return"WII";switch(static_cast<uint32_t>(id>>32)){case 0x00050000:return"GAME";case 0x0005000C:return"DLC";case 0x0005000E:return"UPDATE";default:return"SYSTEM_OR_OTHER";}}
std::string Region(const std::string&r){if(r.empty())return"ALL";unsigned v=std::strtoul(r.c_str(),nullptr,0)&7U;switch(v){case 1:return"JPN";case 2:return"USA";case 3:return"JPN/USA";case 4:return"EUR";case 5:return"EUR/JPN";case 6:return"EUR/USA";default:return"ALL";}}

bool WriteMetaSample(const std::string& xml, std::string& error){
    constexpr const char* path="/vol/external01/wiiu/apps/GameVault-WiiU-Exporter/meta_sample.xml";
    errno=0;FILE*f=std::fopen(path,"wb");
    if(!f){error=std::string("meta sample fopen: ")+std::strerror(errno);return false;}
    const size_t written=std::fwrite(xml.data(),1,xml.size(),f);
    if(written!=xml.size()){error=std::string("meta sample fwrite: ")+std::strerror(errno);std::fclose(f);return false;}
    if(std::fflush(f)!=0){error=std::string("meta sample fflush: ")+std::strerror(errno);std::fclose(f);return false;}
    if(std::fclose(f)!=0){error=std::string("meta sample fclose: ")+std::strerror(errno);return false;}
    return true;
}
}
bool ScanInstalledTitles(std::vector<InstalledTitle>&titles,std::string&error,std::vector<std::string>&diagnostics){titles.clear();error.clear();diagnostics.clear();bool sampleWritten=false;int32_t h=MCP_Open();if(h<0){error="MCP_Open failed";return false;}MCPError cr=MCP_TitleCount(h);if(cr<0){MCP_Close(h);error="MCP_TitleCount failed";return false;}uint32_t count=static_cast<uint32_t>(cr);std::vector<MCPTitleListType> r(count);uint32_t fetched=count;if(count&&MCP_TitleList(h,&fetched,r.data(),r.size()*sizeof(MCPTitleListType))<0){MCP_Close(h);error="MCP_TitleList failed";return false;}MCP_Close(h);r.resize(fetched);titles.reserve(fetched);for(const auto&s:r){uint32_t p=static_cast<uint32_t>(s.titleId>>32);if(p!=0x00050000&&p!=0x0005000C&&p!=0x0005000E)continue;InstalledTitle t;t.titleId=s.titleId;t.version=s.titleVersion;t.path=s.path;t.groupId=s.groupId;t.appType=static_cast<uint32_t>(s.appType);t.osVersion=s.osVersion;t.sdkVersion=s.sdkVersion;t.indexedDevice=s.indexedDevice;t.storage=t.indexedDevice=="usb"?"USB":(t.indexedDevice=="mlc"?"NAND":"UNKNOWN");t.kind=Kind(t.titleId,t.appType);std::string b=Base(s),m=b+"/meta/meta.xml",xml;unsigned attempt=0;int oe=0;bool ok=ReadRetry(m,xml,attempt,oe);if(diagnostics.size()<8){char line[220];if(ok)std::snprintf(line,sizeof(line),"META OK attempt %u",attempt);else std::snprintf(line,sizeof(line),"META FAIL after 10 attempts: %d %s",oe,std::strerror(oe));diagnostics.emplace_back(line);diagnostics.emplace_back("PATH: "+m);}t.name=Tag(xml,"longname_en");if(t.name.empty())t.name=Tag(xml,"shortname_en");if(t.name.empty()){char id[17];std::snprintf(id,sizeof(id),"%016" PRIX64,t.titleId);t.name=id;}t.productCode=Tag(xml,"product_code");t.region=Region(Tag(xml,"region"));t.iconPath=b+"/meta/iconTex.tga";
        t.publisher=Tag(xml,"publisher_en");
        t.companyCode=Tag(xml,"company_code");
        t.contentPlatform=Tag(xml,"content_platform");
        t.xmlTitleVersion=UIntTag(xml,"title_version");
        t.drcUse=UIntTag(xml,"drc_use");
        t.networkUse=UIntTag(xml,"network_use");
        t.onlineAccountUse=UIntTag(xml,"online_account_use");
        if(false&&!sampleWritten&&ok&&t.kind=="GAME"){
            std::string sampleError;
            if(WriteMetaSample(xml,sampleError)){
                char id[17];std::snprintf(id,sizeof(id),"%016" PRIX64,t.titleId);
                char info[160];std::snprintf(info,sizeof(info),"SAMPLE %s: %u bytes",id,static_cast<unsigned>(xml.size()));diagnostics.emplace_back(info);
                diagnostics.emplace_back(std::string("longname_en: ")+(Tag(xml,"longname_en").empty()?"NOT FOUND":"FOUND"));
                diagnostics.emplace_back(std::string("shortname_en: ")+(Tag(xml,"shortname_en").empty()?"NOT FOUND":"FOUND"));
                diagnostics.emplace_back(std::string("product_code: ")+(Tag(xml,"product_code").empty()?"NOT FOUND":"FOUND"));
                diagnostics.emplace_back(std::string("region: ")+(Tag(xml,"region").empty()?"NOT FOUND":"FOUND"));
                diagnostics.emplace_back("meta_sample.xml written to SD");
                sampleWritten=true;
            }else diagnostics.emplace_back("SAMPLE FAILED: "+sampleError);
        }
        titles.push_back(std::move(t));}return true;}
