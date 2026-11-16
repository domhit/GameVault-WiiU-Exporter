#include <whb/proc.h>
#include <coreinit/screen.h>
#include <coreinit/memdefaultheap.h>
#include <vpad/input.h>
#include <mocha/mocha.h>
#include <cstdio>
#include <string>
#include <vector>
#include "TitleScanner.hpp"
#include "JsonExporter.hpp"

namespace {
void* tv = nullptr;
void* drc = nullptr;
void InitScreen(){OSScreenInit();tv=MEMAllocFromDefaultHeapEx(OSScreenGetBufferSizeEx(SCREEN_TV),0x100);drc=MEMAllocFromDefaultHeapEx(OSScreenGetBufferSizeEx(SCREEN_DRC),0x100);OSScreenSetBufferEx(SCREEN_TV,tv);OSScreenSetBufferEx(SCREEN_DRC,drc);OSScreenEnableEx(SCREEN_TV,true);OSScreenEnableEx(SCREEN_DRC,true);}
void Draw(const std::vector<std::string>& lines){for(int x=SCREEN_TV;x<=SCREEN_DRC;++x){auto screen=static_cast<OSScreenID>(x);OSScreenClearBufferEx(screen,0x101820FF);uint32_t y=2;for(const auto& line:lines)OSScreenPutFontEx(screen,2,y++,line.c_str());OSScreenFlipBuffersEx(screen);}}
void EndScreen(){OSScreenEnableEx(SCREEN_TV,false);OSScreenEnableEx(SCREEN_DRC,false);if(tv)MEMFreeToDefaultHeap(tv);if(drc)MEMFreeToDefaultHeap(drc);tv=nullptr;drc=nullptr;}
std::string MochaStatus(const char* label,MochaUtilsStatus status){char text[160];std::snprintf(text,sizeof(text),"%s: %d %s",label,static_cast<int>(status),Mocha_GetStatusStr(status));return text;}
}

int main(int,char**){
    WHBProcInit();
    VPADInit();
    InitScreen();
    Draw({"GameVault Wii U Exporter v0.1.0-dev3","","Initializing Mocha..."});

    const MochaUtilsStatus initStatus=Mocha_InitLibrary();
    const MochaUtilsStatus usbStatus=initStatus==MOCHA_RESULT_SUCCESS?Mocha_MountFS("storage_usb01",nullptr,"/vol/storage_usb01"):MOCHA_RESULT_LIB_UNINITIALIZED;
    const MochaUtilsStatus mlcStatus=initStatus==MOCHA_RESULT_SUCCESS?Mocha_MountFS("storage_mlc01",nullptr,"/vol/storage_mlc01"):MOCHA_RESULT_LIB_UNINITIALIZED;

    std::vector<std::string> screen={"GameVault Wii U Exporter v0.1.0-dev3","MOUNT DIAGNOSTICS",MochaStatus("INIT",initStatus),MochaStatus("USB",usbStatus),MochaStatus("MLC",mlcStatus),"","Scanning installed titles..."};
    Draw(screen);

    std::vector<InstalledTitle> titles;
    std::vector<std::string> diagnostics;
    std::string error;
    bool scanOk=ScanInstalledTitles(titles,error,diagnostics);
    bool exportOk=false;
    if(scanOk)exportOk=ExportJson(titles,error);

    screen={"GameVault Wii U Exporter v0.1.0-dev3","MOUNT DIAGNOSTICS",MochaStatus("INIT",initStatus),MochaStatus("USB",usbStatus),MochaStatus("MLC",mlcStatus),""};
    for(const auto& line:diagnostics)screen.push_back(line);
    screen.push_back("");
    if(!scanOk)screen.push_back(std::string("SCAN FAILED: ")+error);
    else if(!exportOk)screen.push_back(std::string("EXPORT FAILED: ")+error);
    else {char result[96];std::snprintf(result,sizeof(result),"EXPORT OK: %u titles",static_cast<unsigned>(titles.size()));screen.push_back(result);}
    screen.push_back("Press A or HOME to test minimal exit");
    Draw(screen);

    while(WHBProcIsRunning()){
        VPADStatus status{};VPADReadError readError{};
        if(VPADRead(VPAD_CHAN_0,&status,1,&readError)>0&&(status.trigger&VPAD_BUTTON_A))break;
    }

    Draw({"GameVault Wii U Exporter v0.1.0-dev3","","EXIT STEP: WHBProcShutdown only"});
    EndScreen();
    // Diagnostic exit: deliberately no Mocha_UnmountFS, Mocha_DeInitLibrary or VPADShutdown.
    WHBProcShutdown();
    return 0;
}
