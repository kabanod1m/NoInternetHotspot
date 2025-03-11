// NoInternetHotspot.cpp : Defines the entry point for the application.
//

#include "Main.h"
#include "NoInternetHotspotWnd.h"

int APIENTRY wWinMain (
        _In_ HINSTANCE hInstance,
        _In_opt_ HINSTANCE hPrevInstance,
        _In_ LPWSTR lpCmdLine,
        _In_ int nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize the Windows Runtime.
    Microsoft::WRL::Wrappers::RoInitializeWrapper initialize(RO_INIT_MULTITHREADED);
    if (FAILED(initialize))
    {
        MessageBox(nullptr, _T("Failed to initialize Windows Runtime!"), STR_FATAL_ERROR, MB_OK);
        return static_cast<HRESULT>(initialize);
    }

    NoInternetHotspotWnd wnd(hInstance, hPrevInstance, lpCmdLine, nCmdShow);

    return wnd.MsgLoop();
}

// debug, also to make the console visible do: Project -> Properties -> Configuration Properties -> Linker -> System -> SubSystem = "Console" (instead of "Windows")
int main () {
    return wWinMain(GetModuleHandle(nullptr), nullptr, GetCommandLine(), SW_SHOW);
}
