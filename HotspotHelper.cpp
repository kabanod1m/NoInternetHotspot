// this is all actually a bit altered SimpleConsole.cpp from Microsoft's WiFiDirectLegacyAPDemo

#include "HotspotHelper.h"

HotspotHelper::HotspotHelper () : _apEvent(CreateEventEx(nullptr, nullptr, 0, WRITE_OWNER | EVENT_ALL_ACCESS)) {
    HRESULT hr = _apEvent.IsValid() ? S_OK : HRESULT_FROM_WIN32(GetLastError());
    if (FAILED(hr)) {
        std::wcout << "Failed to create AP event: " << hr << std::endl;
        throw WlanHostedNetworkException("Create event failed", hr);
    }

    _hostedNetwork.RegisterListener(this);
    _hostedNetwork.RegisterPrompt(this);
}

HotspotHelper::~HotspotHelper () {
    _hostedNetwork.RegisterListener(nullptr);
    _hostedNetwork.RegisterPrompt(nullptr);
}

void HotspotHelper::OnDeviceConnected (std::basic_string<TCHAR> remoteHostName) {
    //std::wcout << std::endl << "Peer connected: " << remoteHostName << std::endl;
    
}

void HotspotHelper::OnDeviceDisconnected (std::basic_string<TCHAR> deviceId) {
    //std::wcout << std::endl << "Peer disconnected: " << deviceId << std::endl;

}

void HotspotHelper::OnAdvertisementStarted () {
    /*std::wcout << "Soft AP started!" << std::endl
        << "Peers can connect to: " << _hostedNetwork.GetSSID() << std::endl
        << "Passphrase: " << _hostedNetwork.GetPassphrase() << std::endl;
    SetEvent(_apEvent.Get());*/

}

void HotspotHelper::OnAdvertisementStopped (std::basic_string<TCHAR> message) {
    /*std::wcout << "Soft AP stopped." << std::endl;
    SetEvent(_apEvent.Get());*/

}

void HotspotHelper::OnAdvertisementAborted (std::basic_string<TCHAR> message) {
    /*std::wcout << "Soft AP aborted: " << message << std::endl;
    SetEvent(_apEvent.Get());*/

}

void HotspotHelper::OnAsyncException (std::basic_string<TCHAR> message) {
    /*std::wcout << std::endl << "Caught exception in asynchronous method: " << message << std::endl;*/

}

void HotspotHelper::LogMessage (std::basic_string<TCHAR> message) {
    /*std::wcout << std::endl << message << std::endl;*/

}

bool HotspotHelper::AcceptIncommingConnection () {
    /*std::wcout << std::endl << "Accept peer connection? (y/n)" << std::endl;

    std::basic_string<TCHAR> response;
    getline(std::wcin, response);

    if (response.length() > 0 &&
        (response[0] == 'y' || response[0] == 'Y'))
    {
        return true;
    }

    return false;*/
    return false;
}
