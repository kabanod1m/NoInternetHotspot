// this is all actually a bit altered SimpleConsole.h from Microsoft's WiFiDirectLegacyAPDemo
#pragma once

#include "WlanHostedNetworkWinRT.h"

class HotspotHelper : public IWlanHostedNetworkListener, public IWlanHostedNetworkPrompt {
	public:
	HotspotHelper ();
	virtual ~HotspotHelper ();
	
	// IWlanHostedNetworkListener Implementation
    virtual void OnDeviceConnected (std::basic_string<TCHAR> remoteHostName) override;
    virtual void OnDeviceDisconnected (std::basic_string<TCHAR> deviceId) override;
    virtual void OnAdvertisementStarted () override;
    virtual void OnAdvertisementStopped (std::basic_string<TCHAR> message) override;
    virtual void OnAdvertisementAborted (std::basic_string<TCHAR> message) override;
    virtual void OnAsyncException (std::basic_string<TCHAR> message) override;
    virtual void LogMessage (std::basic_string<TCHAR> message) override;
    // IWlanHostedNetworkPrompt Implementation
    virtual bool AcceptIncommingConnection () override;

    private:
    WlanHostedNetworkHelper _hostedNetwork;
    Microsoft::WRL::Wrappers::Event _apEvent; // Event helper to wait on async operations
};