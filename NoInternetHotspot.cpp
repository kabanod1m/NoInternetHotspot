// NoInternetHotspot.cpp : Defines the entry point for the application.
//

#include "NoInternetHotspot.h"

#include "HotspotHelper.h"
HotspotHelper hotspot;

int APIENTRY wWinMain (
        _In_ HINSTANCE hInstance,
        _In_opt_ HINSTANCE hPrevInstance,
        _In_ LPWSTR lpCmdLine,
        _In_ int nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_NOINTERNETHOTSPOT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance(hInstance, nCmdShow)) {
        return FALSE;
    }

    // Initialize the Windows Runtime.
    Microsoft::WRL::Wrappers::RoInitializeWrapper initialize(RO_INIT_MULTITHREADED);
    if (FAILED(initialize))
    {
        MessageBox(hMain, _T("Failed to initialize Windows Runtime!"), STR_FATAL_ERROR, MB_OK);
        return static_cast<HRESULT>(initialize);
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_NOINTERNETHOTSPOT));

    // Main message loop:
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}

//
//  FUNCTION: MyRegisterClass ()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass (HINSTANCE hInstance) {
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_NOINTERNETHOTSPOT));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCE(IDC_NOINTERNETHOTSPOT);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance (HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance (HINSTANCE hInstance, int nCmdShow) {
    hInst = hInstance; // Store instance handle in our global variable

    // default window size, client area DEFAULT_WINDOWWIDTH x DEFAULT_WINDOWHEIGHT
    RECT window_rect;
    window_rect.left = 0;
    window_rect.top = 0;
    window_rect.right = DEFAULT_WINDOWWIDTH;
    window_rect.bottom = DEFAULT_WINDOWHEIGHT;
    AdjustWindowRect(&window_rect, WS_OVERLAPPEDWINDOW, 0);

    HWND hWnd = CreateWindow(
        szWindowClass,
        szTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        0,
        window_rect.right - window_rect.left,
        window_rect.bottom - window_rect.top,
        nullptr,
        nullptr,
        hInstance,
        nullptr);

    if (!hWnd) {
       return FALSE;
    }

    hMain = hWnd;
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

//
//  FUNCTION: WndProc (HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE:
            LayOutUI(hWnd);
            break;
        case WM_COMMAND: {
                int wmId = LOWORD(wParam);
                // Parse the menu selections:
                switch (wmId) {
                    case IDM_HIDE:
                        if (IsDlgButtonChecked(hWnd, IDM_HIDE)) {
                            CheckDlgButton(hWnd, IDM_HIDE, BST_UNCHECKED);
                            SendMessage(hPass, EM_SETPASSWORDCHAR, 0, 0);
                            RedrawWindow(hPass, nullptr, nullptr, RDW_INVALIDATE);
                        } else {
                            CheckDlgButton(hWnd, IDM_HIDE, BST_CHECKED);
                            SendMessage(hPass, EM_SETPASSWORDCHAR, '*', 0);
                            RedrawWindow(hPass, nullptr, nullptr, RDW_INVALIDATE);
                        }
                        break;
                    case IDM_CTRL:
                        if (hotspotStarted) { // turn off hotspot
                            
                        } else { // turn on hotspot
                            EnableWindow(hCtrl, FALSE);
                            EnableWindow(hSsid, FALSE);
                            EnableWindow(hPass, FALSE);
                            hotspotStarted = TRUE;
                            
                        }
                        break;
                    case IDM_AUTO:
                        if (autoAccept) {
                            CheckDlgButton(hWnd, IDM_AUTO, BST_UNCHECKED);
                            autoAccept = FALSE;
                        } else {
                            CheckDlgButton(hWnd, IDM_AUTO, BST_CHECKED);
                            autoAccept = TRUE;
                        }
                        break;
                    case IDM_LOG:
                        MessageBox(hWnd, _T("not implemented"), _T("Log"), MB_OK);
                        break;
                    case IDM_ABOUT:
                        DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                        break;
                    case IDM_EXIT:
                        DestroyWindow(hWnd);
                        break;
                    default:
                        return DefWindowProc(hWnd, message, wParam, lParam);
                }
            }
            break;
        case WM_PAINT: {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hWnd, &ps);
                //TCHAR test[] = _T("test"); TextOut(hdc, 258, 12, test, _tcslen(test));
                EndPaint(hWnd, &ps);
            }
            break;
        case WM_DESTROY:
            // don't forget to shut off hotspot, wait for it to shut off in the main thread, just shut it off
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    UNREFERENCED_PARAMETER(lParam);
    switch (message) {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

// UI code
void LayOutUI (HWND hWnd) {
    // generating and storing a random value for later usages
    int random = (LONG_PTR)&random % 10000;
    TCHAR srandom[5];
    _stprintf_s(srandom, _T("%04d"), random);
    // default input values
    TCHAR ssidField[
#if MAX_SSIDLENGTH > MAX_COMPUTERNAME_LENGTH
        MAX_SSIDLENGTH
#else
        MAX_COMPUTERNAME_LENGTH
#endif
            + 1];
    TCHAR passwordField[MAX_PASSWORDLENGTH + 1];
    //if config { (read from config here)
    
    //else { (initialize default input values here)
        // default "SSID (Wi-Fi Network name)" value
        DWORD bufCharCount =
#if MAX_SSIDLENGTH > MAX_COMPUTERNAME_LENGTH
            MAX_SSIDLENGTH;
#else
            MAX_COMPUTERNAME_LENGTH;
#endif
        if (!GetComputerName(ssidField, &bufCharCount)) {
            StringCchCopy(ssidField, sizeof(ssidField) / sizeof(ssidField[0]), _T("NoInternet"));
            StringCchCat(ssidField, sizeof(ssidField) / sizeof(ssidField[0]), srandom); // later this will get saved to a config file so that the user won't have to manually connect every time
        }
        // default "Wi-Fi Network password" value
        StringCchCopy(passwordField, sizeof(passwordField) / sizeof(passwordField[0]), _T("SharedPassword"));
        StringCchCat(passwordField, sizeof(passwordField) / sizeof(passwordField[0]), srandom);

    // "SSID (Wi-Fi Network name)" label
    CreateWindow(_T("Static"), _T("Wi-Fi Network name"), WS_VISIBLE | WS_CHILD, 0, 0, 256, 14, hWnd, nullptr, nullptr, nullptr);
    // "SSID (W-Fi Network name)" input box
    hSsid = CreateWindow(_T("Edit"), ssidField, WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 0, 14, 256, 20, hWnd, nullptr, nullptr, nullptr);
    SendMessage(hSsid, EM_LIMITTEXT, MAX_SSIDLENGTH, 0);
    // "Wi-Fi Network password" label
    CreateWindow(_T("Static"), _T("Wi-Fi Network password"), WS_VISIBLE | WS_CHILD, 0, 35, 208, 14, hWnd, nullptr, nullptr, nullptr);
    // "Hide" checkbox
    hHide = CreateWindow(_T("Button"), _T("Hide"), WS_VISIBLE | WS_CHILD | BS_CHECKBOX, 208, 35, 48, 14, hWnd, (HMENU)IDM_HIDE, hInst, nullptr);
    // "Wi-Fi Network password" input box
    hPass = CreateWindow(_T("Edit"), passwordField, WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 0, 49, 256, 20, hWnd, nullptr, nullptr, nullptr);
    SendMessage(hPass, EM_LIMITTEXT, MAX_PASSWORDLENGTH, 0);
    // "Start"(/"Stop") button
    hCtrl = CreateWindow(_T("Button"), _T("Start"), WS_VISIBLE | WS_CHILD, 0, 70, 64, 20, hWnd, (HMENU)IDM_CTRL, nullptr, nullptr); 
    // "Auto-accept" checkbox
    hAuto = CreateWindow(_T("Button"), _T("Auto-accept"), WS_VISIBLE | WS_CHILD | BS_CHECKBOX, 65, 70, 191, 20, hWnd, (HMENU)IDM_AUTO, hInst, nullptr);
    if (autoAccept) CheckDlgButton(hWnd, IDM_AUTO, BST_CHECKED);
    // "Currently connected:" label
    CreateWindow(_T("Static"), _T("Currently connected:"), WS_VISIBLE | WS_CHILD, 0, 90, 256, 14, hWnd, nullptr, nullptr, nullptr);
    // "Currently connected:" listbox
    hList = CreateWindowEx(WS_EX_CLIENTEDGE, _T("Listbox"), nullptr, WS_VISIBLE | WS_CHILD | WS_VSCROLL | ES_AUTOVSCROLL, 0, 104, 256, 132, hWnd, nullptr, nullptr, nullptr);
}

// debug, also to make the console visible do: Project -> Properties -> Configuration Properties -> Linker -> System -> SubSystem = "Console" (instead of "Windows")
int main () {
    return wWinMain(GetModuleHandle(nullptr), nullptr, GetCommandLine(), SW_SHOW);
}
