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
#include "VWiiScanner.hpp"

namespace {
void DrawConsole() {
    WHBLogConsoleDraw();
}
}

int main(int, char**) {
    WHBProcInit();
    WHBLogConsoleInit();
    WHBLogConsoleSetColor(0x000000FF);
    VPADInit();

    WHBLogPrintf("GameVault Wii U Exporter v0.2.0-dev2");
    WHBLogPrintf("");
    WHBLogPrintf("Scanning Wii U inventory...");
    DrawConsole();

    const MochaUtilsStatus initStatus = Mocha_InitLibrary();
    const MochaUtilsStatus usbStatus = initStatus == MOCHA_RESULT_SUCCESS
        ? Mocha_MountFS("storage_usb01", nullptr, "/vol/storage_usb01")
        : MOCHA_RESULT_LIB_UNINITIALIZED;
    const MochaUtilsStatus mlcStatus = initStatus == MOCHA_RESULT_SUCCESS
        ? Mocha_MountFS("storage_mlc01", nullptr, "/vol/storage_mlc01")
        : MOCHA_RESULT_LIB_UNINITIALIZED;

    if (initStatus != MOCHA_RESULT_SUCCESS || usbStatus != MOCHA_RESULT_SUCCESS || mlcStatus != MOCHA_RESULT_SUCCESS) {
        WHBLogPrintf("Storage warning: INIT=%d USB=%d MLC=%d", static_cast<int>(initStatus), static_cast<int>(usbStatus), static_cast<int>(mlcStatus));
    }

    std::vector<InstalledTitle> titles;
    std::vector<std::string> diagnostics;
    std::string error;
    const bool scanOk = ScanInstalledTitles(titles, error, diagnostics);
    const bool exportOk = scanOk && ExportJson(titles, error);
    std::string vwiiError;
    const VWiiScanResult vwii = ScanVWiiTitles();
    const bool vwiiExportOk = ExportVWiiDiagnostics(vwii, vwiiError);

    (void)diagnostics;
    if (!scanOk) {
        WHBLogPrintf("ERROR: Scan failed");
        WHBLogPrintf("%s", error.c_str());
    } else if (!exportOk) {
        WHBLogPrintf("ERROR: Export failed");
        WHBLogPrintf("%s", error.c_str());
    } else {
        unsigned games = 0, updates = 0, dlc = 0;
        for (const auto& title : titles) {
            if (title.kind == "GAME") ++games;
            else if (title.kind == "UPDATE") ++updates;
            else if (title.kind == "DLC") ++dlc;
        }
        WHBLogPrintf("Export complete");
        WHBLogPrintf("");
        WHBLogPrintf("Games:   %u", games);
        WHBLogPrintf("Updates: %u", updates);
        WHBLogPrintf("DLC:     %u", dlc);
        WHBLogPrintf("Total:   %u", static_cast<unsigned>(titles.size()));
        WHBLogPrintf("");
        WHBLogPrintf("installed_titles.json written to SD");
        WHBLogPrintf("");
        if (vwii.available && vwiiExportOk) {
            WHBLogPrintf("vWii diagnostic: %u titles", static_cast<unsigned>(vwii.titles.size()));
            WHBLogPrintf("vwii_diagnostics.json written to SD");
        } else {
            WHBLogPrintf("vWii diagnostic unavailable");
            WHBLogPrintf("mount=%d %s", vwii.mountStatus, vwii.mountStatusText.c_str());
            const std::string detail = vwii.error.empty() ? vwiiError : vwii.error;
            WHBLogPrintf("%s", detail.c_str());
        }
    }
    WHBLogPrintf("");
    WHBLogPrintf("Press A to return to Wii U Menu");
    DrawConsole();

    bool menuLaunchRequested = false;
    while (WHBProcIsRunning()) {
        if (!menuLaunchRequested) {
            VPADStatus status{};
            VPADReadError readError{};
            if (VPADRead(VPAD_CHAN_0, &status, 1, &readError) > 0 && (status.trigger & VPAD_BUTTON_A)) {
                WHBLogPrintf("Returning to Wii U Menu...");
                DrawConsole();
                SYSLaunchMenu();
                menuLaunchRequested = true;
            }
        }
        // WHBLogConsole handles ProcUI ACQUIRE/RELEASE and draws only while foreground is owned.
        DrawConsole();
    }

    // ProcUI-aware console cleanup. Mocha mounts are intentionally left to process teardown.
    VPADShutdown();
    WHBLogConsoleFree();
    WHBProcShutdown();
    return 0;
}
