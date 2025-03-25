#include "NoInternetHotspotWnd.h"

// Global Constants:
const TCHAR empty_string[1] = _T("");

// Global Variables:
NoInternetHotspotWnd* nihwnd;
HANDLE startstopthread; DWORD startstopthreadid;
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
bool hotspot_started = FALSE;
bool logfirstentry = TRUE;

// UI Global Variables:
HWND hMain;
HWND hSsid;
HWND hPass;
HWND hHide;
HWND hCtrl;
HWND hAuto;
HWND hList;
HWND hLog;
HWND hLblLog;
HWND hSave;
HWND hClear;

NoInternetHotspotWnd::NoInternetHotspotWnd (_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) : _apEvent(CreateEventEx(nullptr, nullptr, 0, WRITE_OWNER | EVENT_ALL_ACCESS)) {
    nihwnd = this;

    std::wstring wCmdLineStr = std::wstring(lpCmdLine);// wCmdLineStr = std::wstring(L"/start");
    std::vector<std::wstring> argns;
    std::vector<std::wstring> argvs;
    size_t posslash;
    while ((posslash = wCmdLineStr.find('/')) != std::wstring::npos) {
        ParseArg(&wCmdLineStr, posslash, &argns, &argvs);
    }
    ParseArg(&wCmdLineStr, -1, &argns, &argvs);
    argns.erase(argns.begin());
    argvs.erase(argvs.begin());

    // Attempt loading saved user preferences from the glorious Windows Registry
    cfgloadsuccess = ReadConfig(&cfg);
    if (!cfgloadsuccess) { // default values for the config
        // generating and storing a random value for later usages
        int random = (LONG_PTR)&random % 10000;
        TCHAR srandom[5];
        _stprintf_s(srandom, _T("%04d"), random);
        // default "Wi-Fi Network name" value
        DWORD bufCharCount =
#if MAX_SSIDLENGTH > MAX_COMPUTERNAME_LENGTH
            MAX_SSIDLENGTH;
#else
            MAX_COMPUTERNAME_LENGTH;
#endif
        if (!GetComputerName(cfg.ssid, &bufCharCount)) {
            StringCchCopy(cfg.ssid, sizeof(cfg.ssid) / sizeof(cfg.ssid[0]), _T("NoInternet"));
            StringCchCat(cfg.ssid, sizeof(cfg.ssid) / sizeof(cfg.ssid[0]), srandom); // later this will get saved to a config file so that the user won't have to manually connect every time
        }
        // default "Wi-Fi Network password" value
        StringCchCopy(cfg.pass, sizeof(cfg.pass) / sizeof(cfg.pass[0]), _T("SharedPassword"));
        StringCchCat(cfg.pass, sizeof(cfg.pass) / sizeof(cfg.pass[0]), srandom);
        // default "Hide" checkbox value
        cfg.hidepass = FALSE;
        // default "Auto-accept" checkbox value
        cfg.autoaccept = FALSE;
    }
    // Override loaded values with values from launch parameters


    // Perform AP initialization
    HRESULT hr = _apEvent.IsValid() ? S_OK : HRESULT_FROM_WIN32(GetLastError());
    if (FAILED(hr))
    {
        std::wcout << "Failed to create AP event: " << hr << std::endl;
        throw WlanHostedNetworkException("Create event failed", hr);
    }
    _hostedNetwork.RegisterListener(this);
    _hostedNetwork.RegisterPrompt(this);

    //  Windows app intialization
    // Initialize global strings
    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_NOINTERNETHOTSPOT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);
    // Perform application initialization
    if (!InitInstance(hInstance, nCmdShow)) {
        MessageBox(nullptr, _T("Failed to initialize the main window!"), STR_FATAL_ERROR, MB_OK);
        return;
    }
    hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_NOINTERNETHOTSPOT));

    // Handle config
    if (nihwnd->cfgloadsuccess) {
        if (nihwnd->cfg.hidepass) {
            SendMessage(hPass, EM_SETPASSWORDCHAR, '*', 0);
            RedrawWindow(hPass, nullptr, nullptr, RDW_INVALIDATE);
        }
        if (nihwnd->cfg.hidepass) CheckDlgButton(hMain, IDM_HIDE, BST_CHECKED);
        if (nihwnd->cfg.autoaccept) nihwnd->SwitchAutoaccept(TRUE);
        if (!nihwnd->cfgloadsuccess) nihwnd->LogMessage(std::wstring(L"Couldn't load preferences from ") + STR_CFG_REGPATH + std::wstring(L" ; it's to be expected if the program was started for the first time."));
    }

    // Handle command line actions
    for (int i = 0; i < argns.size(); i++) {
        //LogMessage(L"ARGN:" + argns[i]);
        //LogMessage(L"ARGV:" + argvs[i]);
        static const std::wstring lpmmsg = std::wstring(STR_LPM);
        if (argns[i] == L"ssid") {
            SendMessage(hSsid, WM_SETTEXT, 0, (LPARAM)argvs[i].c_str());

            LogMessage(lpmmsg + STR_LPM_SSID + argvs[i]);
        } else if (argns[i] == L"pass") {
            SendMessage(hPass, WM_SETTEXT, 0, (LPARAM)argvs[i].c_str());

            LogMessage(lpmmsg + STR_LPM_PASS + argvs[i]);
        } else if (argns[i] == L"hide") {
            std::transform(argvs[i].begin(), argvs[i].end(), argvs[i].begin(), towlower);
            if (argvs[i] != L"false") {
                CheckDlgButton(hMain, IDM_HIDE, BST_CHECKED);
                SendMessage(hPass, EM_SETPASSWORDCHAR, '*', 0);
                RedrawWindow(hPass, nullptr, nullptr, RDW_INVALIDATE);

                LogMessage(lpmmsg + STR_LPM_HIDE + L"to true");
            } else {
                CheckDlgButton(hMain, IDM_HIDE, BST_UNCHECKED);
                SendMessage(hPass, EM_SETPASSWORDCHAR, 0, 0);
                RedrawWindow(hPass, nullptr, nullptr, RDW_INVALIDATE);

                LogMessage(lpmmsg + STR_LPM_HIDE + L"to false");
            }
        } else if (argns[i] == L"auto") {
            if (argvs[i] != L"false") {
                SwitchAutoaccept(TRUE);

                LogMessage(lpmmsg + STR_LPM_AUTO + L"to true");
            } else {
                SwitchAutoaccept(FALSE);

                LogMessage(lpmmsg + STR_LPM_AUTO + L"to false");
            }
        } else if (argns[i] == L"start") {
            SendMessage(hMain, WM_COMMAND, MAKEWPARAM(IDM_CTRL, 0), 0);

            LogMessage(L"As specified by launch parameters, starting Wi-Fi immediately");
        } else {
            LogMessage(L"Unrecognized launch parameter: " + argns[i] + L" " + argvs[i]);
        }
    }
}

NoInternetHotspotWnd::~NoInternetHotspotWnd () {
    _hostedNetwork.RegisterListener(nullptr);
    _hostedNetwork.RegisterPrompt(nullptr);
}

void NoInternetHotspotWnd::ParseArg (std::wstring* CmdString, size_t inSlashPos, std::vector<std::wstring>* outArgns, std::vector<std::wstring>* outArgvs) { // use inSlashPos = -1 to not erase
    static size_t posspace;
    static std::wstring argn;
    std::wstring argv = L"";
    argn = CmdString->substr(0, inSlashPos);
    if ((posspace = argn.find(' ')) != std::wstring::npos) {
        argv = argn.substr(posspace + 1, argn.length() - (posspace + 1) - ((inSlashPos == -1) ? 0 : 1));
        argn = argn.substr(0, posspace);
    }
    outArgns->push_back(argn);
    outArgvs->push_back(argv);
    CmdString->erase(0, inSlashPos + 1);
}

//
//  FUNCTION: MyRegisterClass ()
//
//  PURPOSE: Registers the window class.
//
ATOM NoInternetHotspotWnd::MyRegisterClass (HINSTANCE hInstance) {
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_NOINTERNETHOTSPOT));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCE(IDC_NOINTERNETHOTSPOT);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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
BOOL NoInternetHotspotWnd::InitInstance (HINSTANCE hInstance, int nCmdShow) {
    hInst = hInstance; // Store instance handle in our global variable

    // default window size, client area DEFAULT_WINDOWWIDTH x DEFAULT_WINDOWHEIGHT
    
    RECT default_window_rect;
    default_window_rect.left = 0;
    default_window_rect.top = 0;
    default_window_rect.right = DEFAULT_WINDOWWIDTH;
    default_window_rect.bottom = DEFAULT_WINDOWHEIGHT;
    AdjustWindowRect(&default_window_rect, WS_OVERLAPPEDWINDOW, 0);
#pragma warning(suppress: 4244) // respectfully screen resolutions won't ever be larger than short limit? >32767
    adjframewidth = default_window_rect.right - default_window_rect.left - DEFAULT_WINDOWWIDTH;
#pragma warning(suppress: 4244) // respectfully screen resolutions won't ever be larger than short limit? >32767
    adjframeheight = default_window_rect.bottom - default_window_rect.top - DEFAULT_WINDOWHEIGHT;

    HWND hWnd = CreateWindow(
        szWindowClass,
        szTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        0,
        adjframewidth,
        adjframeheight,
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

int NoInternetHotspotWnd::MsgLoop () {
    // Main message loop:
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg) && !IsDialogMessage(hMain, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return (int)msg.wParam;
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
LRESULT CALLBACK NoInternetHotspotWnd::WndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE:
            LayOutUI(hWnd);
            break;
        case WM_SIZE: {
                RECT window_rect;
                if (!GetWindowRect(hMain, &window_rect)) {
                    break;
                }
                MoveWindow(hList, GUI_LIS_LIST_X, GUI_LIS_LIST_Y, GUI_LIS_LIST_W, window_rect.bottom - window_rect.top - nihwnd->adjframeheight - (DEFAULT_WINDOWHEIGHT - GUI_LIS_LIST_H), FALSE);
#pragma warning(suppress: 4244) // respectfully screen resolutions won't ever be larger than short limit? >32767
                short logwidth = (window_rect.right - window_rect.left < DEFAULT_LOGWINDOWWIDTH) ? GUI_TXT_LOG_W : window_rect.right - window_rect.left - nihwnd->adjframewidth - (DEFAULT_LOGWINDOWWIDTH - GUI_TXT_LOG_W);
                MoveWindow(hLblLog, GUI_LBL_LOG_X, GUI_LBL_LOG_Y, logwidth, GUI_LBL_LOG_H, FALSE);
                MoveWindow(hLog, GUI_TXT_LOG_X, GUI_TXT_LOG_Y, logwidth, window_rect.bottom - window_rect.top - nihwnd->adjframeheight - (DEFAULT_WINDOWHEIGHT - GUI_TXT_LOG_H), FALSE);
                MoveWindow(hSave, GUI_BTN_SAVELOG_X, window_rect.bottom - window_rect.top - nihwnd->adjframeheight - DEFAULT_WINDOWHEIGHT + GUI_BTN_SAVELOG_Y, GUI_BTN_SAVELOG_W, GUI_BTN_SAVELOG_H, FALSE);
                MoveWindow(hClear, GUI_BTN_CLEARLOG_X, window_rect.bottom - window_rect.top - nihwnd->adjframeheight - DEFAULT_WINDOWHEIGHT + GUI_BTN_CLEARLOG_Y, GUI_BTN_CLEARLOG_W, GUI_BTN_CLEARLOG_H, FALSE);
            }
            break;
        case WM_CTLCOLORSTATIC:
            if ((HWND)lParam == hLog) {
                return (LRESULT)((HBRUSH)GetStockObject(WHITE_BRUSH));
            }
        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            // Respond to incoming messages:
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
                    if (!hotspot_started) { // turn on hotspot
                        if (GetWindowTextLength(hSsid) == 0 || GetWindowTextLength(hPass) == 0) {
                            MessageBox(hMain, _T("Fields must not be empty!"), STR_WARNING, MB_OK);
                            break;
                        }

                        EnableWindow(hCtrl, FALSE);
                        SendMessage(hSsid, EM_SETREADONLY, TRUE, 0);
                        SendMessage(hPass, EM_SETREADONLY, TRUE, 0);
                        
                        TCHAR* input = GetField(hSsid);
                        nihwnd->_hostedNetwork.SetSSID(input);
                        input = GetField(hPass);
                        nihwnd->_hostedNetwork.SetPassphrase(input);
                        startstopthread = CreateThread(NULL, 0, SwitchHotspot, &hotspot_started, 0, &startstopthreadid);
                        break;
                    } // turn off hotspot
                    EnableWindow(hCtrl, FALSE);

                    startstopthread = CreateThread(NULL, 0, SwitchHotspot, &hotspot_started, 0, &startstopthreadid);
                    break;
                case IDM_AUTO:
                    if (IsDlgButtonChecked(hWnd, IDM_AUTO)) {
                        CheckDlgButton(hWnd, IDM_AUTO, BST_UNCHECKED);
                        nihwnd->_hostedNetwork.SetAutoAccept(FALSE);
                    } else {
                        CheckDlgButton(hWnd, IDM_AUTO, BST_CHECKED);
                        nihwnd->_hostedNetwork.SetAutoAccept(TRUE);
                    }
                    break;
                case IDM_LOG: {
                        RECT window_rect;
                        if (!GetWindowRect(hMain, &window_rect)) {
                            break;
                        }
                        if (window_rect.right - window_rect.left - nihwnd->adjframewidth >= DEFAULT_LOGWINDOWWIDTH) {
                            break;
                        }
                        MoveWindow(hMain, window_rect.left, window_rect.top, DEFAULT_LOGWINDOWWIDTH + nihwnd->adjframewidth, window_rect.bottom - window_rect.top, TRUE);
                    }
                    break;
                case IDM_SAVELOG: {
                        static const std::wstring failmsg = L"Was unable to save the file: ";

                        DWORD loglength = GetWindowTextLength(hLog);
                        if (!loglength) {
                            nihwnd->LogMessage(failmsg + L"The log is empty, there is nothing to save");
                            break;
                        }
                        SIZE_T logtextsize = (loglength + 1) * sizeof(TCHAR);
                        TCHAR* logtext = (TCHAR*)malloc(logtextsize);
                        if (logtext == NULL) {
                            nihwnd->LogMessage(failmsg + L"Out Of Memory");
                            break;
                        }
                        if (!GetWindowText(hLog, logtext, logtextsize)) {
                            nihwnd->LogMessage(failmsg + L"Could not get log's contents");
                            break;
                        }

                        TCHAR filename[MAX_PATH] = _T("");
                        OPENFILENAME filechoice = {0};
                        filechoice.lStructSize = sizeof(filechoice);
                        filechoice.hwndOwner = hWnd;
                        filechoice.lpstrFilter = _T("Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0");
                        filechoice.lpstrFile = filename;
                        filechoice.nMaxFile = MAX_PATH;
                        filechoice.Flags = OFN_HIDEREADONLY | OFN_ENABLESIZING;
                        filechoice.lpstrDefExt = _T("txt");
                        if (!GetSaveFileName(&filechoice)) {
                            nihwnd->LogMessage(failmsg + L"Cancelled by user");
                            break;
                        }
                        HANDLE hOutfile = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                        if (hOutfile == INVALID_HANDLE_VALUE) {
                            free(logtext);
                            nihwnd->LogMessage(failmsg + L"Could not create/open the specified file");
                            break;
                        }
                        DWORD writtenbytes;
                        if (!WriteFile(hOutfile, logtext, logtextsize, &writtenbytes, NULL)) {
                            free(logtext);
                            CloseHandle(hOutfile);
                            nihwnd->LogMessage(failmsg + L"Could not write log's contents to file");
                            break;
                        }
                        free(logtext);
                        CloseHandle(hOutfile);
                        nihwnd->LogMessage(L"Saved log to: " + std::wstring(filename));
                        break;
                    }
                case IDM_CLEARLOG:
                    SendMessage(hLog, WM_SETTEXT, 0, (LRESULT)empty_string);
                    logfirstentry = TRUE;
                    break;
                case IDM_ABOUT:
                    DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                    break;
                case IDM_SOURCE:
                    if (MessageBox(hMain, STR_SOURCELINK, STR_OPENLINK, MB_YESNO) == IDYES) ShellExecute(0, 0, STR_SOURCELINK, 0, 0, SW_SHOW);
                    break;
                case IDM_EXIT:
                    DestroyWindow(hWnd);
                    break;
                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;
        }
        /*case WM_PAINT: {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hWnd, &ps);
                //TCHAR test[] = _T("test"); TextOut(hdc, 258, 12, test, _tcslen(test));
                EndPaint(hWnd, &ps);
            }
            break;*/
        case WM_GETMINMAXINFO: {
                if (nihwnd == nullptr) {
                    break;
                }
                LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
                lpMMI->ptMinTrackSize.x = DEFAULT_WINDOWWIDTH + nihwnd->adjframewidth;
                lpMMI->ptMinTrackSize.y = DEFAULT_WINDOWHEIGHT + nihwnd->adjframeheight;
            }
            break;
        case WM_DESTROY:
            if (hotspot_started) {
                nihwnd->_hostedNetwork.Stop();
                WaitForSingleObjectEx(nihwnd->_apEvent.Get(), MAX_TIMEOUT, FALSE);
            }
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK NoInternetHotspotWnd::About (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
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

// config
bool NoInternetHotspotWnd::ReadConfig (config* ptConfig) {
    unsigned long cfgssidsize = (MAX_SSIDLENGTH + 1) * sizeof(TCHAR);
    unsigned long cfgpasssize = (MAX_PASSWORDLENGTH + 1) * sizeof(TCHAR);
    unsigned long cfghidepasssize = sizeof(ptConfig->hidepass);
    unsigned long cfgautoacceptsize = sizeof(ptConfig->autoaccept);
    HKEY hkSOFTWARE;
    if (    RegOpenKeyEx(HKEY_CURRENT_USER, _T("SOFTWARE"), 0, KEY_READ, &hkSOFTWARE) != ERROR_SUCCESS ||
            RegGetValue(hkSOFTWARE, CFG_NAME, CFG_SSID, RRF_RT_REG_SZ, NULL, ptConfig->ssid, &cfgssidsize) != ERROR_SUCCESS ||
            RegGetValue(hkSOFTWARE, CFG_NAME, CFG_PASS, RRF_RT_REG_SZ, NULL, ptConfig->pass, &cfgpasssize) != ERROR_SUCCESS ||
            RegGetValue(hkSOFTWARE, CFG_NAME, CFG_HIDEPASS, RRF_RT_DWORD, NULL, &(ptConfig->hidepass), &cfghidepasssize) != ERROR_SUCCESS ||
            RegGetValue(hkSOFTWARE, CFG_NAME, CFG_AUTOACCEPT, RRF_RT_DWORD, NULL, &(ptConfig->autoaccept), &cfgautoacceptsize) != ERROR_SUCCESS ||
            RegCloseKey(hkSOFTWARE) != ERROR_SUCCESS) {
        return false;
    }
    // for debug
    /*int ret;
    ret = RegOpenKeyEx(HKEY_CURRENT_USER, _T("SOFTWARE"), 0, KEY_READ, &hkSOFTWARE);
    if (ret != ERROR_SUCCESS) {
        nihwnd->LogMessage(std::wstring(L"1 Couldn't load preferences from ") + STR_CFG_REGPATH + std::wstring(L", error code: ") + std::to_wstring(ret));
        return false;
    }
    ret = RegGetValue(hkSOFTWARE, CFG_NAME, CFG_SSID, RRF_RT_REG_SZ, NULL, ptConfig->ssid, &cfgssidsize);
    if (ret != ERROR_SUCCESS) {
        nihwnd->LogMessage(std::wstring(L"2 Couldn't load preferences from ") + STR_CFG_REGPATH + std::wstring(L", error code: ") + std::to_wstring(ret));
        return false;
    }
    ret = RegGetValue(hkSOFTWARE, CFG_NAME, CFG_PASS, RRF_RT_REG_SZ, NULL, ptConfig->pass, &cfgpasssize);
    if (ret != ERROR_SUCCESS) {
        nihwnd->LogMessage(std::wstring(L"3 Couldn't load preferences from ") + STR_CFG_REGPATH + std::wstring(L", error code: ") + std::to_wstring(ret));
        return false;
    }
    ret = RegGetValue(hkSOFTWARE, CFG_NAME, CFG_HIDEPASS, RRF_RT_DWORD, NULL, &(ptConfig->hidepass), &cfghidepasssize);
    if (ret != ERROR_SUCCESS) {
        nihwnd->LogMessage(std::wstring(L"4 Couldn't load preferences from ") + STR_CFG_REGPATH + std::wstring(L", error code: ") + std::to_wstring(ret));
        return false;
    }
    ret = RegGetValue(hkSOFTWARE, CFG_NAME, CFG_AUTOACCEPT, RRF_RT_DWORD, NULL, &(ptConfig->autoaccept), &cfgautoacceptsize);
    if (ret != ERROR_SUCCESS) {
        nihwnd->LogMessage(std::wstring(L"5 Couldn't load preferences from ") + STR_CFG_REGPATH + std::wstring(L", error code: ") + std::to_wstring(ret));
        return false;
    }
    ret = RegCloseKey(hkSOFTWARE);
    if (ret != ERROR_SUCCESS) {
        nihwnd->LogMessage(std::wstring(L"6 Couldn't load preferences from ") + STR_CFG_REGPATH + std::wstring(L", error code: ") + std::to_wstring(ret));
        return false;
    }*/
    return true;
}

void NoInternetHotspotWnd::WriteConfig (TCHAR* uiSsid, TCHAR* uiPass, DWORD uiHidepass, DWORD uiAutoaccept) {
    HKEY hkSOFTWARE;
    HKEY hkNoInternetHotspot;
    if (    uiSsid == NULL || uiPass == NULL ||
            RegOpenKeyEx(HKEY_CURRENT_USER, _T("SOFTWARE"), 0, KEY_CREATE_SUB_KEY, &hkSOFTWARE) != ERROR_SUCCESS ||
            RegCreateKeyEx(hkSOFTWARE, CFG_NAME, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hkNoInternetHotspot, NULL) != ERROR_SUCCESS ||
            RegSetValueEx(hkNoInternetHotspot, CFG_SSID, 0, REG_SZ, (LPBYTE)uiSsid, ((((DWORD)lstrlen(uiSsid) + 1)) * sizeof(TCHAR))) != ERROR_SUCCESS ||
            RegSetValueEx(hkNoInternetHotspot, CFG_PASS, 0, REG_SZ, (LPBYTE)uiPass, ((((DWORD)lstrlen(uiPass) + 1)) * sizeof(TCHAR))) != ERROR_SUCCESS ||
            RegSetValueEx(hkNoInternetHotspot, CFG_HIDEPASS, 0, REG_DWORD, (BYTE*)&uiHidepass, sizeof(DWORD)) != ERROR_SUCCESS ||
            RegSetValueEx(hkNoInternetHotspot, CFG_AUTOACCEPT, 0, REG_DWORD, (BYTE*)&uiAutoaccept, sizeof(DWORD)) != ERROR_SUCCESS ||
            RegCloseKey(hkNoInternetHotspot) != ERROR_SUCCESS ||
            RegCloseKey(hkSOFTWARE) != ERROR_SUCCESS) {
        nihwnd->LogMessage(std::wstring(L"Couldn't save preferences to ") + STR_CFG_REGPATH);
        return;
    }
    // for debug
    /*int ret;
    ret = RegOpenKeyEx(HKEY_CURRENT_USER, _T("SOFTWARE"), 0, KEY_CREATE_SUB_KEY, &hkSOFTWARE);
    if (ret != ERROR_SUCCESS) {
        nihwnd->LogMessage(std::wstring(L"1 Couldn't save preferences to ") + STR_CFG_REGPATH + std::wstring(L", error code: ") + std::to_wstring(ret));
        return;
    }
    ret = RegCreateKeyEx(hkSOFTWARE, CFG_NAME, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hkNoInternetHotspot, NULL);
    if (ret != ERROR_SUCCESS) {
        nihwnd->LogMessage(std::wstring(L"2 Couldn't save preferences to ") + STR_CFG_REGPATH + std::wstring(L", error code: ") + std::to_wstring(ret));
        return;
    }
    ret = RegSetValueEx(hkNoInternetHotspot, CFG_SSID, 0, REG_SZ, (LPBYTE)uiSsid, ((((DWORD)lstrlen(uiSsid) + 1)) * sizeof(TCHAR)));
    if (ret != ERROR_SUCCESS) {
        nihwnd->LogMessage(std::wstring(L"3 Couldn't save preferences to ") + STR_CFG_REGPATH + std::wstring(L", error code: ") + std::to_wstring(ret));
        return;
    }
    ret = RegSetValueEx(hkNoInternetHotspot, CFG_PASS, 0, REG_SZ, (LPBYTE)uiPass, ((((DWORD)lstrlen(uiPass) + 1)) * sizeof(TCHAR)));
    if (ret != ERROR_SUCCESS) {
        nihwnd->LogMessage(std::wstring(L"4 Couldn't save preferences to ") + STR_CFG_REGPATH + std::wstring(L", error code: ") + std::to_wstring(ret));
        return;
    }
    ret = RegSetValueEx(hkNoInternetHotspot, CFG_HIDEPASS, 0, REG_DWORD, reinterpret_cast<BYTE*>(&uiHidepass), sizeof(bool));
    if (ret != ERROR_SUCCESS) {
        nihwnd->LogMessage(std::wstring(L"5 Couldn't save preferences to ") + STR_CFG_REGPATH + std::wstring(L", error code: ") + std::to_wstring(ret));
        return;
    }
    ret = RegSetValueEx(hkNoInternetHotspot, CFG_AUTOACCEPT, 0, REG_DWORD, reinterpret_cast<BYTE*>(&uiAutoaccept), sizeof(bool));
    if (ret != ERROR_SUCCESS) {
        nihwnd->LogMessage(std::wstring(L"6 Couldn't save preferences to ") + STR_CFG_REGPATH + std::wstring(L", error code: ") + std::to_wstring(ret));
        return;
    }
    ret = RegCloseKey(hkNoInternetHotspot);
    if (ret != ERROR_SUCCESS) {
        nihwnd->LogMessage(std::wstring(L"7 Couldn't save preferences to ") + STR_CFG_REGPATH + std::wstring(L", error code: ") + std::to_wstring(ret));
        return;
    }
    ret = RegCloseKey(hkSOFTWARE);
    if (ret != ERROR_SUCCESS) {
        nihwnd->LogMessage(std::wstring(L"8 Couldn't save preferences to ") + STR_CFG_REGPATH + std::wstring(L", error code: ") + std::to_wstring(ret));
        return;
    }*/
    nihwnd->LogMessage(std::wstring(L"Saved preferences to ") + STR_CFG_REGPATH);
}

DWORD WINAPI NoInternetHotspotWnd::SwitchHotspot (LPVOID lpParam) {
    if (*(bool*)lpParam == FALSE) nihwnd->_hostedNetwork.Start(); else nihwnd->_hostedNetwork.Stop();
    return 0;
}

// UI code
void NoInternetHotspotWnd::LayOutUI (HWND hWnd) {
    // "SSID (Wi-Fi Network name)" label
    CreateWindow(_T("Static"), _T("Wi-Fi Network name"), WS_VISIBLE | WS_CHILD, GUI_LBL_SSID_X, GUI_LBL_SSID_Y, GUI_LBL_SSID_W, GUI_LBL_SSID_H, hWnd, nullptr, nullptr, nullptr);
    // "SSID (W-Fi Network name)" input box
    hSsid = CreateWindow(_T("Edit"), nihwnd->cfg.ssid, WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL | WS_TABSTOP, GUI_INP_SSID_X, GUI_INP_SSID_Y, GUI_INP_SSID_W, GUI_INP_SSID_H, hWnd, nullptr, nullptr, nullptr);
    SendMessage(hSsid, EM_LIMITTEXT, MAX_SSIDLENGTH, 0);
    // "Wi-Fi Network password" label
    CreateWindow(_T("Static"), _T("Wi-Fi Network password"), WS_VISIBLE | WS_CHILD, GUI_LBL_PASS_X, GUI_LBL_PASS_Y, GUI_LBL_PASS_W, GUI_LBL_PASS_H, hWnd, nullptr, nullptr, nullptr);
    // "Wi-Fi Network password" input box
    hPass = CreateWindow(_T("Edit"), nihwnd->cfg.pass, WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL | WS_TABSTOP, GUI_INP_PASS_X, GUI_INP_PASS_Y, GUI_INP_PASS_W, GUI_INP_PASS_WH, hWnd, nullptr, nullptr, nullptr);
    SendMessage(hPass, EM_LIMITTEXT, MAX_PASSWORDLENGTH, 0);
    // "Start"(/"Stop") button
    hCtrl = CreateWindow(_T("Button"), STR_BTN_START, WS_VISIBLE | WS_CHILD | WS_TABSTOP, GUI_BTN_START_X, GUI_BTN_START_Y, GUI_BTN_START_W, GUI_BTN_START_H, hWnd, (HMENU)IDM_CTRL, nullptr, nullptr);
    // "Hide" checkbox
    hHide = CreateWindow(_T("Button"), _T("Hide"), WS_VISIBLE | WS_CHILD | BS_CHECKBOX | WS_TABSTOP, GUI_CHK_HIDE_X, GUI_CHK_HIDE_Y, GUI_CHK_HIDE_W, GUI_CHK_HIDE_H, hWnd, (HMENU)IDM_HIDE, hInst, nullptr);
    // "Auto-accept" checkbox
    hAuto = CreateWindow(_T("Button"), _T("Auto-accept"), WS_VISIBLE | WS_CHILD | BS_CHECKBOX | WS_TABSTOP, GUI_CHK_AUTO_X, GUI_CHK_AUTO_Y, GUI_CHK_AUTO_W, GUI_CHK_AUTO_H, hWnd, (HMENU)IDM_AUTO, hInst, nullptr);
    // "Currently connected:" label
    CreateWindow(_T("Static"), _T("Currently connected:"), WS_VISIBLE | WS_CHILD, GUI_LBL_LIST_X, GUI_LBL_LIST_Y, GUI_LBL_LIST_W, GUI_LBL_LIST_H, hWnd, nullptr, nullptr, nullptr);
    // "Currently connected:" listbox
    hList = CreateWindowEx(WS_EX_CLIENTEDGE, _T("Listbox"), nullptr, WS_VISIBLE | WS_CHILD | WS_VSCROLL | WS_HSCROLL | LBS_DISABLENOSCROLL | ES_AUTOVSCROLL | WS_TABSTOP, GUI_LIS_LIST_X, GUI_LIS_LIST_Y, GUI_LIS_LIST_W, GUI_LIS_LIST_H, hWnd, nullptr, nullptr, nullptr);
    // "Log" label
    hLblLog = CreateWindow(_T("Static"), _T("Log"), WS_VISIBLE | WS_CHILD, GUI_LBL_LOG_X, GUI_LBL_LOG_Y, GUI_LBL_LOG_W, GUI_LBL_LOG_H, hWnd, nullptr, nullptr, nullptr);
    // Log text box
    hLog = CreateWindow(_T("Edit"), nullptr, WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | WS_VSCROLL | ES_AUTOVSCROLL | ES_READONLY, GUI_TXT_LOG_X, GUI_TXT_LOG_Y, GUI_TXT_LOG_W, GUI_TXT_LOG_H, hWnd, nullptr, nullptr, nullptr);
    // "Save to file" button
    hSave = CreateWindow(_T("Button"), _T("Save to File"), WS_VISIBLE | WS_CHILD, GUI_BTN_SAVELOG_X, GUI_BTN_SAVELOG_Y, GUI_BTN_SAVELOG_W, GUI_BTN_SAVELOG_H, hWnd, (HMENU)IDM_SAVELOG, nullptr, nullptr);
    // "Clear log" button
    hClear = CreateWindow(_T("Button"), _T("Clear"), WS_VISIBLE | WS_CHILD, GUI_BTN_CLEARLOG_X, GUI_BTN_CLEARLOG_Y, GUI_BTN_CLEARLOG_W, GUI_BTN_CLEARLOG_H, hWnd, (HMENU)IDM_CLEARLOG, nullptr, nullptr);
}

TCHAR* NoInternetHotspotWnd::GetField (HWND hWnd) {
    TCHAR* tempfieldbuffer = (TCHAR*)malloc(sizeof(TCHAR) * (
#if MAX_SSIDLENGTH > MAX_PASSWORDLENGTH
        MAX_SSIDLENGTH
#else
        MAX_PASSWORDLENGTH
#endif
            + 1));
    if (tempfieldbuffer == NULL) {
        return NULL;
    }
    GetWindowText(hWnd, tempfieldbuffer,
#if MAX_SSIDLENGTH > MAX_PASSWORDLENGTH
        MAX_SSIDLENGTH
#else
        MAX_PASSWORDLENGTH
#endif
            + 1);
    return tempfieldbuffer;
}

void NoInternetHotspotWnd::SwitchAutoaccept (bool turnon) {
    if (turnon) {
        int amougs = CheckDlgButton(hMain, IDM_AUTO, BST_CHECKED);
        _hostedNetwork.SetAutoAccept(TRUE);
        int sus = IsDlgButtonChecked(hMain, IDM_AUTO);
        return;
    }
    CheckDlgButton(hMain, IDM_AUTO, BST_UNCHECKED);
    _hostedNetwork.SetAutoAccept(FALSE);
}

void NoInternetHotspotWnd::OnDeviceConnected (std::wstring deviceId) {
    LogMessage(L"Peer connected: " + deviceId);
    SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)deviceId.c_str());
    SendMessage(hList, LB_SETHORIZONTALEXTENT, DEFAULT_CONNECTEDDEVICESWIDTH_SCROLL, 0);
}

void NoInternetHotspotWnd::OnDeviceDisconnected (std::wstring deviceId) {
    LogMessage(L"Peer disconnected: " + deviceId);
    int index = SendMessage(hList, LB_FINDSTRINGEXACT, -1, (LPARAM)deviceId.c_str());
    SendMessage(hList, LB_DELETESTRING, index, 0);
}

void NoInternetHotspotWnd::OnAdvertisementStarted () {
    SetWindowText(hCtrl, STR_BTN_STOP);
    EnableWindow(hCtrl, TRUE);
    LogMessage(L"Advertisement started");

    SetEvent(_apEvent.Get());
    CloseHandle(startstopthread);
    hotspot_started = true;

    WriteConfig(GetField(hSsid), GetField(hPass), SendMessage(hHide, BM_GETCHECK, 0, 0), SendMessage(hAuto, BM_GETCHECK, 0, 0));
}

void NoInternetHotspotWnd::OnAdvertisementStopped (std::wstring message) {
    SetWindowText(hCtrl, STR_BTN_START);
    EnableWindow(hCtrl, TRUE);
    SendMessage(hSsid, EM_SETREADONLY, FALSE, 0);
    SendMessage(hPass, EM_SETREADONLY, FALSE, 0);
    LogMessage(L"Advertisement Stopped: " + message);

    SetEvent(_apEvent.Get());
    CloseHandle(startstopthread);
    hotspot_started = false;
}

void NoInternetHotspotWnd::OnAdvertisementAborted (std::wstring message) {
    SetWindowText(hCtrl, STR_BTN_START);
    EnableWindow(hCtrl, TRUE);
    SendMessage(hSsid, EM_SETREADONLY, FALSE, 0);
    SendMessage(hPass, EM_SETREADONLY, FALSE, 0);
    std::wstring msg = L"Soft AP aborted: " + message;
    MessageBox(hMain, msg.c_str(), STR_WARNING, MB_OK);
    LogMessage(msg);

    SetEvent(_apEvent.Get());
    CloseHandle(startstopthread);
    hotspot_started = false;
}

void NoInternetHotspotWnd::OnAsyncException (std::wstring message) {
    std::wstring msg = L"Undefined behavior detected. You might need to restart the app; Caught exception in asynchronous method: " + message;
    MessageBox(hMain, msg.c_str(), STR_WARNING, MB_OK);
    LogMessage(msg);

    SetEvent(_apEvent.Get());
    CloseHandle(startstopthread);
    hotspot_started = false;
}

void NoInternetHotspotWnd::LogMessage (std::wstring message) { // adds time into the beginning, newline if needed, and sends it to the edit control (log)
    SYSTEMTIME localtime;
    GetLocalTime(&localtime);
    SIZE_T logentrysize = message.length() + 32;
    TCHAR* logentry = (TCHAR*)malloc(logentrysize * sizeof(TCHAR));
    if (logentry == nullptr) return;
    // if it's the first entry, don't add \r\n in the beginning; if it isn't, always add
    TCHAR newline[3] = _T("");
    if (logfirstentry) {
        logfirstentry = FALSE;
    } else {
        _stprintf_s(newline, 3, _T("%s"), _T("\r\n"));
    }
    _stprintf_s(logentry, logentrysize, _T("%s[%d-%02d-%02d %02d:%02d:%02d] %s"), newline, localtime.wYear, localtime.wMonth, localtime.wDay, localtime.wHour, localtime.wMinute, localtime.wSecond, message.c_str());
    DWORD selstart, selend;
    int end = GetWindowTextLength(hLog);
    SendMessage(hLog, EM_GETSEL, (WPARAM)&selstart, (LPARAM)&selend);
    SendMessage(hLog, EM_SETSEL, end, end);
    SendMessage(hLog, EM_REPLACESEL, TRUE, (LPARAM)logentry);
    SendMessage(hLog, EM_SETSEL, selstart, selend);
    RECT rect;
    GetClientRect(hLog, &rect);
    InvalidateRect(hLog, &rect, TRUE);
    free(logentry);
}

bool NoInternetHotspotWnd::AcceptIncomingConnection () {
    if (MessageBox(hMain, _T("A device wants to connect. Accept?"), STR_WARNING, MB_YESNO) == IDYES) return true;
    return false;
}