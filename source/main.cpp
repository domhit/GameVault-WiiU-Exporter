#include <whb/proc.h>
#include <coreinit/screen.h>
#include <coreinit/memdefaultheap.h>
#include <sysapp/launch.h>
#include <vpad/input.h>
#include <mocha/mocha.h>
#include <cstdio>
#include <string>
#include <vector>
#include "TitleScanner.hpp"
#include "JsonExporter.hpp"
namespace{void*tv=nullptr;void*drc=nullptr;void Init(){OSScreenInit();tv=MEMAllocFromDefaultHeapEx(OSScreenGetBufferSizeEx(SCREEN_TV),0x100);drc=MEMAllocFromDefaultHeapEx(OSScreenGetBufferSizeEx(SCREEN_DRC),0x100);OSScreenSetBufferEx(SCREEN_TV,tv);OSScreenSetBufferEx(SCREEN_DRC,drc);OSScreenEnableEx(SCREEN_TV,true);OSScreenEnableEx(SCREEN_DRC,true);}void Draw(const std::vector<std::string>&l){for(int x=SCREEN_TV;x<=SCREEN_DRC;++x){auto q=(OSScreenID)x;OSScreenClearBufferEx(q,0x101820FF);uint32_t y=2;for(auto&a:l)OSScreenPutFontEx(q,2,y++,a.c_str());OSScreenFlipBuffersEx(q);}}void End(){OSScreenEnableEx(SCREEN_TV,false);OSScreenEnableEx(SCREEN_DRC,false);if(tv)MEMFreeToDefaultHeap(tv);if(drc)MEMFreeToDefaultHeap(drc);}std::string MS(const char*l,MochaUtilsStatus s){char b[128];snprintf(b,sizeof(b),"%s: %d %s",l,(int)s,Mocha_GetStatusStr(s));return b;}}
int main(int,char**){WHBProcInit();VPADInit();Init();auto mi=Mocha_InitLibrary();auto us=mi==MOCHA_RESULT_SUCCESS?Mocha_MountFS("storage_usb01",nullptr,"/vol/storage_usb01"):MOCHA_RESULT_LIB_UNINITIALIZED;auto ms=mi==MOCHA_RESULT_SUCCESS?Mocha_MountFS("storage_mlc01",nullptr,"/vol/storage_mlc01"):MOCHA_RESULT_LIB_UNINITIALIZED;Draw({"GameVault Wii U Exporter v0.1.0-dev4",MS("INIT",mi),MS("USB",us),MS("MLC",ms),"Scanning with retry..."});std::vector<InstalledTitle>t;std::vector<std::string>d;std::string e;bool scan=ScanInstalledTitles(t,e,d),exp=scan&&ExportJson(t,e);std::vector<std::string>o={"GameVault Wii U Exporter v0.1.0-dev4",MS("INIT",mi),MS("USB",us),MS("MLC",ms),""};for(auto&x:d)o.push_back(x);o.push_back("");if(!scan)o.push_back("SCAN FAILED: "+e);else if(!exp)o.push_back("EXPORT FAILED: "+e);else{char b[64];snprintf(b,sizeof(b),"EXPORT OK: %u titles",(unsigned)t.size());o.push_back(b);}o.push_back("A: return to Wii U Menu");o.push_back("HOME: Close Software via ProcUI");Draw(o);bool requested=false;while(WHBProcIsRunning()){if(!requested){VPADStatus s{};VPADReadError r{};if(VPADRead(VPAD_CHAN_0,&s,1,&r)>0&&(s.trigger&VPAD_BUTTON_A)){Draw({"GameVault Wii U Exporter v0.1.0-dev4","Requesting Wii U Menu..."});SYSLaunchMenu();requested=true;}}}End();VPADShutdown();WHBProcShutdown();return 0;}
