#include <whb/proc.h>
#include <whb/log.h>
#include <whb/log_console.h>
#include <sysapp/launch.h>
#include <vpad/input.h>
#include <mocha/mocha.h>
#include <cstdio>
#include <string>
#include <vector>
#include "TitleScanner.hpp"
#include "JsonExporter.hpp"

namespace {
void PrintMochaStatus(const char* label, MochaUtilsStatus status) {
    WHBLogPrintf("%s: %d %s", label, static_cast<int>(status), Mocha_GetStatusStr(status));
}
void DrawConsole() {
    WHBLogConsoleDraw();
}
}

int main(int, char**) {
    WHBProcInit();
    WHBLogConsoleInit();
    VPADInit();

    WHBLogPrintf("GameVault Wii U Exporter v0.1.0-dev7");
    WHBLogPrintf("");
    WHBLogPrintf("Initializing Mocha...");
    DrawConsole();

    const MochaUtilsStatus initStatus = Mocha_InitLibrary();
    const MochaUtilsStatus usbStatus = initStatus == MOCHA_RESULT_SUCCESS
        ? Mocha_MountFS("storage_usb01", nullptr, "/vol/storage_usb01")
        : MOCHA_RESULT_LIB_UNINITIALIZED;
    const MochaUtilsStatus mlcStatus = initStatus == MOCHA_RESULT_SUCCESS
        ? Mocha_MountFS("storage_mlc01", nullptr, "/vol/storage_mlc01")
        : MOCHA_RESULT_LIB_UNINITIALIZED;

    PrintMochaStatus("INIT", initStatus);
    PrintMochaStatus("USB", usbStatus);
    PrintMochaStatus("MLC", mlcStatus);
    WHBLogPrintf("Scanning installed titles with metadata retry...");
    DrawConsole();

    std::vector<InstalledTitle> titles;
    std::vector<std::string> diagnostics;
    std::string error;
    const bool scanOk = ScanInstalledTitles(titles, error, diagnostics);
    const bool exportOk = scanOk && ExportJson(titles, error);

    for (const auto& line : diagnostics) WHBLogPrintf("%s", line.c_str());
    if (!scanOk) {
        WHBLogPrintf("SCAN FAILED: %s", error.c_str());
    } else if (!exportOk) {
        WHBLogPrintf("EXPORT FAILED: %s", error.c_str());
    } else {
        WHBLogPrintf("EXPORT OK: %u titles", static_cast<unsigned>(titles.size()));
    }
    WHBLogPrintf("");
    WHBLogPrintf("A: return to Wii U Menu");
    WHBLogPrintf("HOME: Close Software via ProcUI");
    DrawConsole();

    bool menuLaunchRequested = false;
    while (WHBProcIsRunning()) {
        if (!menuLaunchRequested) {
            VPADStatus status{};
            VPADReadError readError{};
            if (VPADRead(VPAD_CHAN_0, &status, 1, &readError) > 0 && (status.trigger & VPAD_BUTTON_A)) {
                WHBLogPrintf("Requesting Wii U Menu...");
                DrawConsole();
                SYSLaunchMenu();
                menuLaunchRequested = true;
            }
        }
        // WHBLogConsole handles ProcUI ACQUIRE/RELEASE and draws only while foreground is owned.
        DrawConsole();
    }

    // Keep Mocha unmount/deinit disabled in dev5 to isolate the ProcUI/console transition.
    VPADShutdown();
    WHBLogConsoleFree();
    WHBProcShutdown();
    return 0;
}
