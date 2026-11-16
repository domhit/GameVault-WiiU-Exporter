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
void* tvBuffer = nullptr;
void* drcBuffer = nullptr;

void InitScreen() {
    OSScreenInit();
    tvBuffer = MEMAllocFromDefaultHeapEx(OSScreenGetBufferSizeEx(SCREEN_TV), 0x100);
    drcBuffer = MEMAllocFromDefaultHeapEx(OSScreenGetBufferSizeEx(SCREEN_DRC), 0x100);
    OSScreenSetBufferEx(SCREEN_TV, tvBuffer);
    OSScreenSetBufferEx(SCREEN_DRC, drcBuffer);
    OSScreenEnableEx(SCREEN_TV, true);
    OSScreenEnableEx(SCREEN_DRC, true);
}
void Draw(const std::vector<std::string>& lines) {
    for (int screen = SCREEN_TV; screen <= SCREEN_DRC; ++screen) {
        OSScreenClearBufferEx(static_cast<OSScreenID>(screen), 0x101820FF);
        uint32_t y = 2;
        for (const auto& line : lines) OSScreenPutFontEx(static_cast<OSScreenID>(screen), 2, y++, line.c_str());
        OSScreenFlipBuffersEx(static_cast<OSScreenID>(screen));
    }
}
void ShutdownScreen() {
    OSScreenEnableEx(SCREEN_TV, false); OSScreenEnableEx(SCREEN_DRC, false);
    if (tvBuffer) MEMFreeToDefaultHeap(tvBuffer);
    if (drcBuffer) MEMFreeToDefaultHeap(drcBuffer);
}
}

int main(int, char**) {
    WHBProcInit(); VPADInit(); InitScreen();
    Draw({"GameVault Wii U Exporter v0.1.0-dev1", "", "Initializing Mocha..."});
    if (Mocha_InitLibrary() != MOCHA_RESULT_SUCCESS) {
        Draw({"GameVault Wii U Exporter", "", "ERROR: Mocha_InitLibrary failed", "Press HOME to exit"});
    } else {
        const bool usbMounted = Mocha_MountFS("storage_usb01", nullptr, "/vol/storage_usb01") == MOCHA_RESULT_SUCCESS;
        const bool mlcMounted = Mocha_MountFS("storage_mlc01", nullptr, "/vol/storage_mlc01") == MOCHA_RESULT_SUCCESS;
        Draw({"GameVault Wii U Exporter v0.1.0-dev1", "", "Scanning installed titles..."});
        std::vector<InstalledTitle> titles; std::string error;
        if (!ScanInstalledTitles(titles, error)) {
            Draw({"GameVault Wii U Exporter", "", "SCAN FAILED", error, "", "Press A or HOME to exit"});
        } else if (!ExportJson(titles, error)) {
            Draw({"GameVault Wii U Exporter", "", "EXPORT FAILED", error, "", "Press A or HOME to exit"});
        } else {
            unsigned games=0, updates=0, dlc=0;
            for (const auto& t : titles) { if(t.kind=="GAME") ++games; else if(t.kind=="UPDATE") ++updates; else if(t.kind=="DLC") ++dlc; }
            char totalLine[64], countsLine[96];
            std::snprintf(totalLine,sizeof(totalLine),"Export successful: %u titles",static_cast<unsigned>(titles.size()));
            std::snprintf(countsLine,sizeof(countsLine),"Games: %u   Updates: %u   DLC: %u",games,updates,dlc);
            Draw({"GameVault Wii U Exporter v0.1.0-dev1", "", totalLine, countsLine, "", "installed_titles.json written to SD", "Press A or HOME to exit"});
        }
        while (WHBProcIsRunning()) {
            VPADStatus status{}; VPADReadError readError{};
            if (VPADRead(VPAD_CHAN_0, &status, 1, &readError) > 0 && (status.trigger & VPAD_BUTTON_A)) break;
        }
        if (usbMounted) Mocha_UnmountFS("storage_usb01");
        if (mlcMounted) Mocha_UnmountFS("storage_mlc01");
        Mocha_DeInitLibrary();
    }
    ShutdownScreen(); VPADShutdown(); WHBProcShutdown();
    return 0;
}
