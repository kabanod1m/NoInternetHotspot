#pragma once

// limits
#define MAX_LOADSTRING 100
#define MAX_SSIDLENGTH 32
#define MAX_PASSWORDLENGTH 63

// strings
#define STR_FATAL_ERROR _T("Fatal!")

// default values
#define DEFAULT_WINDOWWIDTH 256
#define DEFAULT_WINDOWHEIGHT 256

// includes
#include "resource.h"
#include "framework.h"

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void				LayOutUI(HWND);

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
bool hotspotStarted = FALSE;
bool autoAccept = FALSE;
WlanHostedNetworkHelper _hostedNetwork;

// UI Global Variables:
HWND hMain;
HWND hSsid;
HWND hPass;
HWND hHide;
HWND hCtrl;
HWND hAuto;
HWND hList;

typedef struct DataFromUI {
    TCHAR ssid[MAX_SSIDLENGTH + 1];
    TCHAR password[MAX_PASSWORDLENGTH + 1];
} UIDATA, * PUIDATA;