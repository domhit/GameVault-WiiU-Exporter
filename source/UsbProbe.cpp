// SPDX-License-Identifier: GPL-2.0-or-later
#include "UsbProbe.hpp"
#include <iosuhax.h>
#include <coreinit/memdefaultheap.h>
#include <coreinit/cache.h>
#include <cstdio>
#include <cerrno>
#include <cstring>

namespace {
constexpr const char* Out="/vol/external01/wiiu/apps/GameVault-WiiU-Exporter/vwii_usb_raw_diagnostics.json";
constexpr const char* Tmp="/vol/external01/wiiu/apps/GameVault-WiiU-Exporter/vwii_usb_raw_diagnostics.json.tmp";
void JsonString(FILE*f,const std::string&v){fputc('"',f);for(unsigned char c:v){switch(c){case'"':fputs("\\\"",f);break;case'\\':fputs("\\\\",f);break;case'\n':fputs("\\n",f);break;case'\r':fputs("\\r",f);break;case'\t':fputs("\\t",f);break;default:if(c<0x20)fprintf(f,"\\u%04X",c);else fputc(c,f);}}fputc('"',f);}
std::string Hex(const unsigned char*b,size_t n){static const char*h="0123456789ABCDEF";std::string o;o.reserve(n*2);for(size_t i=0;i<n;++i){o.push_back(h[b[i]>>4]);o.push_back(h[b[i]&15]);}return o;}
std::string Signature(const unsigned char*b){char s[17]{};for(int i=0;i<16;++i)s[i]=(b[i]>=32&&b[i]<127)?char(b[i]):'.';return s;}
bool HasText(const unsigned char*b,size_t n,const char*s){size_t l=std::strlen(s);if(!l||l>n)return false;for(size_t i=0;i+l<=n;++i)if(std::memcmp(b+i,s,l)==0)return true;return false;}
void ProbeOne(int fsa,const char*dev,UsbProbeResult&r){UsbRawProbe p;p.devicePath=dev;int h=-1;p.rawOpenStatus=IOSUHAX_FSA_RawOpen(fsa,dev,&h);p.rawHandle=h;if(p.rawOpenStatus>=0){void*mem=MEMAllocFromDefaultHeapEx(0x200,0x40);if(mem){std::memset(mem,0,0x200);DCFlushRange(mem,0x200);p.readStatus=IOSUHAX_FSA_RawRead(fsa,mem,0x200,1,0,h);DCInvalidateRange(mem,0x200);if(p.readStatus>=0){auto*b=static_cast<unsigned char*>(mem);p.readable=true;p.signature=Signature(b);p.mbrSignature=b[510]==0x55&&b[511]==0xAA;p.fatLike=HasText(b,0x200,"FAT32")||HasText(b,0x200,"FAT16")||HasText(b,0x200,"MSDOS");p.wbfsLike=(b[0]=='W'&&b[1]=='B'&&b[2]=='F'&&b[3]=='S');p.sector0Hex=Hex(b,0x80);}MEMFreeToDefaultHeap(mem);}IOSUHAX_FSA_RawClose(fsa,h);}r.devices.push_back(std::move(p));}
}

UsbProbeResult ProbeUsbDevices(){UsbProbeResult r;if(IOSUHAX_Open(nullptr)<0)return r;int fsa=IOSUHAX_FSA_Open();if(fsa<0){IOSUHAX_Close();return r;}const char* devices[]={"/dev/usb01","/dev/usb02","/dev/usb03","/dev/usb04","/dev/usb05","/dev/usb06","/dev/ums0","/dev/ums1","/dev/ums2","/dev/ums3"};for(const char*d:devices)ProbeOne(fsa,d,r);IOSUHAX_FSA_Close(fsa);IOSUHAX_Close();return r;}

bool ExportUsbRawDiagnostics(const UsbProbeResult&r,std::string&error){error.clear();FILE*f=fopen(Tmp,"wb");if(!f){error=std::string("fopen: ")+strerror(errno);return false;}fputs("{\n  \"schemaVersion\": 1,\n  \"exporterVersion\": \"0.2.0-dev7\",\n  \"diagnostic\": \"WII_USB_RAW_PROBE\",\n  \"devices\": [\n",f);for(size_t i=0;i<r.devices.size();++i){const auto&p=r.devices[i];fputs("    {\"devicePath\": ",f);JsonString(f,p.devicePath);fprintf(f,", \"rawOpenStatus\": %d, \"rawHandle\": %d, \"readStatus\": %d, \"readable\": %s, \"signature\": ",p.rawOpenStatus,p.rawHandle,p.readStatus,p.readable?"true":"false");JsonString(f,p.signature);fprintf(f,", \"mbrSignature\": %s, \"fatLike\": %s, \"wbfsLike\": %s, \"sector0Hex\": ",p.mbrSignature?"true":"false",p.fatLike?"true":"false",p.wbfsLike?"true":"false");JsonString(f,p.sector0Hex);fprintf(f,"}%s\n",i+1<r.devices.size()?",":"");}fputs("  ]\n}\n",f);if(fflush(f)||ferror(f)){error="write failed";fclose(f);remove(Tmp);return false;}if(fclose(f)){error="close failed";remove(Tmp);return false;}remove(Out);if(rename(Tmp,Out)){error="rename failed";remove(Tmp);return false;}return true;}
