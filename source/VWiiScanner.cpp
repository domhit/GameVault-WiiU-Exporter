#include "VWiiScanner.hpp"
#include <mocha/mocha.h>
#include <coreinit/filesystem_fsa.h>
#include <cstdio>
#include <cerrno>
#include <cstring>

namespace {
constexpr const char* OutputPath="/vol/external01/wiiu/apps/GameVault-WiiU-Exporter/vwii_fsa_diagnostics.json";
constexpr const char* TempPath="/vol/external01/wiiu/apps/GameVault-WiiU-Exporter/vwii_fsa_diagnostics.json.tmp";
void JsonString(FILE*f,const std::string&v){fputc('"',f);for(unsigned char c:v){switch(c){case'"':fputs("\\\"",f);break;case'\\':fputs("\\\\",f);break;case'\n':fputs("\\n",f);break;case'\r':fputs("\\r",f);break;case'\t':fputs("\\t",f);break;default:if(c<0x20)fprintf(f,"\\u%04X",c);else fputc(c,f);}}fputc('"',f);}
std::string FsaText(FSError status){const char*text=FSAGetStatusStr(status);return text?text:"unknown";}
void Probe(FSAClientHandle client,const char*path,VWiiScanResult&result){FSAProbeRow row;row.path=path;FSADirectoryHandle handle{};const FSError status=FSAOpenDir(client,path,&handle);row.openStatus=static_cast<int32_t>(status);row.statusText=FsaText(status);row.opened=static_cast<int32_t>(status)==0;if(row.opened)FSACloseDir(client,handle);result.probes.push_back(std::move(row));}
}

VWiiScanResult ScanVWiiTitles(){
 VWiiScanResult result;
 const MochaUtilsStatus mount=Mocha_MountFS("storage_slccmpt01",nullptr,"/vol/storage_slccmpt01");
 result.mountStatus=static_cast<int32_t>(mount);result.mountStatusText=Mocha_GetStatusStr(mount);
 const FSError init=FSAInit();result.fsaInitStatus=static_cast<int32_t>(init);
 const FSAClientHandle client=FSAAddClient(nullptr);result.clientHandle=static_cast<int32_t>(client);
 if(client<0){result.error="FSAAddClient failed";return result;}
 Probe(client,"/vol/storage_slccmpt01",result);
 Probe(client,"/vol/storage_slccmpt01/title",result);
 Probe(client,"/vol/storage_slccmpt01/ticket",result);
 Probe(client,"/vol/storage_slccmpt01/shared1",result);
 Probe(client,"/vol/storage_slccmpt01/sys",result);
 result.available=result.probes.size()>1&&result.probes[1].opened;
 FSADelClient(client);
 return result;
}

bool ExportVWiiDiagnostics(const VWiiScanResult&r,std::string&error){
 error.clear();FILE*f=fopen(TempPath,"wb");if(!f){error=std::string("fopen: ")+strerror(errno);return false;}
 fputs("{\n  \"schemaVersion\": 1,\n  \"exporterVersion\": \"0.2.0-dev3\",\n  \"diagnostic\": \"VWII_FSA_PROBE\",\n",f);
 fprintf(f,"  \"available\": %s,\n  \"mountStatus\": %d,\n  \"mountStatusText\": ",r.available?"true":"false",r.mountStatus);JsonString(f,r.mountStatusText);
 fprintf(f,",\n  \"fsaInitStatus\": %d,\n  \"clientHandle\": %d,\n  \"error\": ",r.fsaInitStatus,r.clientHandle);JsonString(f,r.error);fputs(",\n  \"probes\": [\n",f);
 for(size_t i=0;i<r.probes.size();++i){const auto&p=r.probes[i];fputs("    {\"path\": ",f);JsonString(f,p.path);fprintf(f,", \"openStatus\": %d, \"statusText\": ",p.openStatus);JsonString(f,p.statusText);fprintf(f,", \"opened\": %s}%s\n",p.opened?"true":"false",i+1<r.probes.size()?",":"");}
 fputs("  ]\n}\n",f);if(fflush(f)||ferror(f)){error=std::string("write: ")+strerror(errno);fclose(f);remove(TempPath);return false;}if(fclose(f)){error=std::string("close: ")+strerror(errno);remove(TempPath);return false;}remove(OutputPath);if(rename(TempPath,OutputPath)){error=std::string("rename: ")+strerror(errno);remove(TempPath);return false;}return true;
}
