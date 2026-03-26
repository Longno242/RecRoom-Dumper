#include <windows.h>
#include <shlobj.h>
#include <string>
#include "dumper.h"
#include "console_ui.h"

#define _CRT_SECURE_NO_WARNINGS

DWORD WINAPI DumpThread(LPVOID lpParam) {
    Sleep(2000);

    char desktop_path[MAX_PATH];
    HRESULT result = SHGetFolderPathA(NULL, CSIDL_DESKTOP, NULL, SHGFP_TYPE_CURRENT, desktop_path);

    std::string dump_path;
    if (SUCCEEDED(result)) {
        dump_path = std::string(desktop_path) + "\\GameDump";
    }
    else {
        dump_path = "C:\\GameDump";
    }
    ConsoleUI::ShowDumperUI([&]() {
        GameDumper::DumpAll(dump_path, [](const std::string& msg) {
            });
        });
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        AllocConsole();
        FILE* fp;
        freopen_s(&fp, "CONOUT$", "w", stdout);
        freopen_s(&fp, "CONIN$", "r", stdin);
        SetConsoleTitleA("RRID Dumper Pro");
        CreateThread(nullptr, 0, DumpThread, nullptr, 0, nullptr);
        break;

    case DLL_PROCESS_DETACH:
        FreeConsole();
        break;
    }
    return TRUE;
}