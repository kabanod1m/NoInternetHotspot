#pragma once

// includes
#include "resource.h"
#include "framework.h"
#include "WlanHostedNetworkWinRT.h"

// limits
#define MAX_TIMEOUT 30000 // milliseconds
#define MAX_LOADSTRING 100
#define MAX_SSIDLENGTH 32
#define MAX_PASSWORDLENGTH 63

// strings
#define STR_FATAL_ERROR _T("Fatal!")
#define STR_WARNING _T("Warning!")
#define STR_OPENLINK _T("Open link?")
#define STR_SOURCELINK _T("https://github.com/kabanod1m/NoInternetHotspot")
#define STR_LPM L"As specified by launch parameters, setting: "
#define STR_LPM_SSID L"Network name field to "
#define STR_LPM_PASS L"Password field to "
#define STR_LPM_HIDE L"Hide checkbox "
#define STR_LPM_AUTO L"Auto-Accept checkbox "
#define STR_BTN_START _T("Start")
#define STR_BTN_STOP _T("Stop")
#define STR_CFG_REGPATH L"HKEY_CURRENT_USER\\SOFTWARE\\NoInternetHotspot"
#define CFG_NAME _T("NoInternetHotspot")
#define CFG_SSID _T("LastSsid")
#define CFG_PASS _T("LastPassword")
#define CFG_HIDEPASS _T("HidePassword")
#define CFG_AUTOACCEPT _T("AutoAccept")

// default values
#define DEFAULT_WINDOWWIDTH 256
#define DEFAULT_WINDOWHEIGHT 256
#define DEFAULT_CONNECTEDDEVICESWIDTH_SCROLL 512
#define DEFAULT_LOGWINDOWWIDTH 512

// ui layout coordinates
#define GUI_LBL_SSID_X 0 // "SSID (Wi-Fi Network name)" label
#define GUI_LBL_SSID_Y 0 // "SSID (Wi-Fi Network name)" label
#define GUI_LBL_SSID_W 256 // "SSID (Wi-Fi Network name)" label
#define GUI_LBL_SSID_H 14 // "SSID (Wi-Fi Network name)" label
#define GUI_INP_SSID_X 0 // "SSID (W-Fi Network name)" input box
#define GUI_INP_SSID_Y 14 // "SSID (W-Fi Network name)" input box
#define GUI_INP_SSID_W 256 // "SSID (W-Fi Network name)" input box
#define GUI_INP_SSID_H 20 // "SSID (W-Fi Network name)" input box
#define GUI_LBL_PASS_X 0 // "Wi-Fi Network password" label
#define GUI_LBL_PASS_Y 35 // "Wi-Fi Network password" label
#define GUI_LBL_PASS_W 208 // "Wi-Fi Network password" label
#define GUI_LBL_PASS_H 14 // "Wi-Fi Network password" label
#define GUI_CHK_HIDE_X 208 // "Hide" checkbox
#define GUI_CHK_HIDE_Y 35 // "Hide" checkbox
#define GUI_CHK_HIDE_W 48 // "Hide" checkbox
#define GUI_CHK_HIDE_H 14 // "Hide" checkbox
#define GUI_INP_PASS_X 0 // "Wi-Fi Network password" input box
#define GUI_INP_PASS_Y 49 // "Wi-Fi Network password" input box
#define GUI_INP_PASS_W 256 // "Wi-Fi Network password" input box
#define GUI_INP_PASS_WH 20 // "Wi-Fi Network password" input box
#define GUI_BTN_START_X 0 // "Start"(/"Stop") button
#define GUI_BTN_START_Y 70 // "Start"(/"Stop") button
#define GUI_BTN_START_W 64 // "Start"(/"Stop") button
#define GUI_BTN_START_H 20 // "Start"(/"Stop") button
#define GUI_CHK_AUTO_X 65 // "Auto-accept" checkbox
#define GUI_CHK_AUTO_Y 70 // "Auto-accept" checkbox
#define GUI_CHK_AUTO_W 191 // "Auto-accept" checkbox
#define GUI_CHK_AUTO_H 20 // "Auto-accept" checkbox
#define GUI_LBL_LIST_X 0 // "Currently connected:" label
#define GUI_LBL_LIST_Y 90 // "Currently connected:" label
#define GUI_LBL_LIST_W 256 // "Currently connected:" label
#define GUI_LBL_LIST_H 14 // "Currently connected:" label
#define GUI_LIS_LIST_X 0 // "Currently connected:" listbox
#define GUI_LIS_LIST_Y 104 // "Currently connected:" listbox
#define GUI_LIS_LIST_W 256 // "Currently connected:" listbox
#define GUI_LIS_LIST_H 132 // "Currently connected:" listbox
#define GUI_LBL_LOG_X 256 // "Log" label
#define GUI_LBL_LOG_Y 0 // "Log" label
#define GUI_LBL_LOG_W 256 // "Log" label
#define GUI_LBL_LOG_H 14 // "Log" label
#define GUI_TXT_LOG_X 256 // Log text box
#define GUI_TXT_LOG_Y 14 // Log text box
#define GUI_TXT_LOG_W 256 // Log text box
#define GUI_TXT_LOG_H 201 // Log text box
#define GUI_BTN_SAVELOG_X 256 // "Save to file" button
#define GUI_BTN_SAVELOG_Y 216 // "Save to file" button
#define GUI_BTN_SAVELOG_W 96 // "Save to file" button
#define GUI_BTN_SAVELOG_H 20 // "Save to file" button
#define GUI_BTN_CLEARLOG_X 352 // "Clear log" button
#define GUI_BTN_CLEARLOG_Y 216 // "Clear log" button
#define GUI_BTN_CLEARLOG_W 64 // "Clear log" button
#define GUI_BTN_CLEARLOG_H 20 // "Clear log" button

struct config {
	TCHAR ssid[MAX_SSIDLENGTH + 1];
	TCHAR pass[MAX_PASSWORDLENGTH + 1];
	DWORD hidepass;
	DWORD autoaccept;
};

class NoInternetHotspotWnd : public IWlanHostedNetworkListener, public IWlanHostedNetworkPrompt {
	public:
	bool cfgloadsuccess = FALSE;
	config cfg;

	NoInternetHotspotWnd (_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow);
	virtual ~NoInternetHotspotWnd ();
	int MsgLoop ();

	// IWlanHostedNetworkListener Implementation
	virtual void OnDeviceConnected (std::wstring deviceId) override;
	virtual void OnDeviceDisconnected (std::wstring deviceId) override;
	virtual void OnAdvertisementStarted () override;
	virtual void OnAdvertisementStopped (std::wstring message) override;
	virtual void OnAdvertisementAborted (std::wstring message) override;
	virtual void OnAsyncException (std::wstring message) override;
	virtual void LogMessage (std::wstring message) override;
	// IWlanHostedNetworkPrompt Implementation
	virtual bool AcceptIncomingConnection () override;

	private:
	HACCEL hAccelTable;
	short adjframewidth;
	short adjframeheight;

	WlanHostedNetworkHelper _hostedNetwork;
	Microsoft::WRL::Wrappers::Event _apEvent; // Event helper to wait on async operations

	static void ParseArg (std::wstring* inCmdString, size_t inSlashPos, std::vector<std::wstring>* outArgns, std::vector<std::wstring>* outArgvs);
	ATOM MyRegisterClass (HINSTANCE hInstance);
	BOOL InitInstance (HINSTANCE, int);
	static LRESULT CALLBACK WndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK About (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	static void LayOutUI (HWND hWnd);
	static bool ReadConfig (config* ptConfig);
	static void WriteConfig (TCHAR* uissid, TCHAR* uipass, DWORD uihidepass, DWORD uiatoaccept);
	static DWORD WINAPI SwitchHotspot (LPVOID lpParam);

	static TCHAR* GetField (HWND hWnd);
};
