// SPDX-License-Identifier: GPL-2.0-or-later
// vWii read-only IOSUHAX/FSA access follows the established access model used
// by Compat Title Installer / wuphax. This file contains no install/write path.
#include "VWiiScanner.hpp"
#include <iosuhax.h>
#include <cstdio>
#include <cerrno>
#include <cstring>

namespace {
constexpr const char* OutputPath="/vol/external01/wiiu/apps/GameVault-WiiU-Exporter/vwii_iosuhax_diagnostics.json";
constexpr const char* TempPath="/vol/external01/wiiu/apps/GameVault-WiiU-Exporter/vwii_iosuhax_diagnostics.json.tmp";
void JsonString(FILE*f,const std::string&v){fputc('"',f);for(unsigned char c:v){switch(c){case'"':fputs("\\\"",f);break;case'\\':fputs("\\\\",f);break;case'\n':fputs("\\n",f);break;case'\r':fputs("\\r",f);break;case'\t':fputs("\\t",f);break;default:if(c<0x20)fprintf(f,"\\u%04X",c);else fputc(c,f);}}fputc('"',f);}
void Probe(int fsa,const char*path,VWiiScanResult&r){IOSUHAXProbeRow row;row.path=path;int handle=-1;row.openStatus=IOSUHAX_FSA_OpenDir(fsa,path,&handle);row.opened=row.openStatus>=0;if(row.opened)IOSUHAX_FSA_CloseDir(fsa,handle);r.probes.push_back(std::move(row));}
}

VWiiScanResult ScanVWiiTitles(){
 VWiiScanResult r;
 r.iosuhaxMcpStatus=IOSUHAX_Open("/dev/mcp");
 if(r.iosuhaxMcpStatus<0){r.iosuhaxDefaultStatus=IOSUHAX_Open(nullptr);if(r.iosuhaxDefaultStatus<0){r.error="IOSUHAX_Open failed for /dev/mcp and /dev/iosuhax";return r;}}
 r.fsaHandle=IOSUHAX_FSA_Open();if(r.fsaHandle<0){r.error="IOSUHAX_FSA_Open failed";IOSUHAX_Close();return r;}
 r.mountStatus=IOSUHAX_FSA_Mount(r.fsaHandle,r.devicePath.c_str(),r.volumePath.c_str(),FSA_MOUNTFLAGS_GLOBAL,nullptr,0);
 // An already mounted volume may return an error; probing remains authoritative.
 Probe(r.fsaHandle,"/vol/storage_slccmpt01",r);
 Probe(r.fsaHandle,"/vol/storage_slccmpt01/title",r);
 Probe(r.fsaHandle,"/vol/storage_slccmpt01/ticket",r);
 Probe(r.fsaHandle,"/vol/storage_slccmpt01/shared1",r);
 Probe(r.fsaHandle,"/vol/storage_slccmpt01/sys",r);
 r.available=r.probes.size()>1&&r.probes[1].opened;
 IOSUHAX_FSA_Close(r.fsaHandle);IOSUHAX_Close();
 return r;
}

bool ExportVWiiDiagnostics(const VWiiScanResult&r,std::string&error){
 error.clear();FILE*f=fopen(TempPath,"wb");if(!f){error=std::string("fopen: ")+strerror(errno);return false;}
 fputs("{\n  \"schemaVersion\": 1,\n  \"exporterVersion\": \"0.2.0-dev4\",\n  \"diagnostic\": \"VWII_IOSUHAX_FSA_PROBE\",\n",f);
 fprintf(f,"  \"available\": %s,\n  \"backend\": ",r.available?"true":"false");JsonString(f,r.backend);
 fprintf(f,",\n  \"iosuhaxMcpStatus\": %d,\n  \"iosuhaxDefaultStatus\": %d,\n  \"fsaHandle\": %d,\n  \"mountStatus\": %d,\n  \"devicePath\": ",r.iosuhaxMcpStatus,r.iosuhaxDefaultStatus,r.fsaHandle,r.mountStatus);JsonString(f,r.devicePath);
 fputs(",\n  \"volumePath\": ",f);JsonString(f,r.volumePath);fputs(",\n  \"error\": ",f);JsonString(f,r.error);fputs(",\n  \"probes\": [\n",f);
 for(size_t i=0;i<r.probes.size();++i){const auto&p=r.probes[i];fputs("    {\"path\": ",f);JsonString(f,p.path);fprintf(f,", \"openStatus\": %d, \"opened\": %s}%s\n",p.openStatus,p.opened?"true":"false",i+1<r.probes.size()?",":"");}
 fputs("  ]\n}\n",f);if(fflush(f)||ferror(f)){error=std::string("write: ")+strerror(errno);fclose(f);remove(TempPath);return false;}if(fclose(f)){error=std::string("close: ")+strerror(errno);remove(TempPath);return false;}remove(OutputPath);if(rename(TempPath,OutputPath)){error=std::string("rename: ")+strerror(errno);remove(TempPath);return false;}return true;
}
