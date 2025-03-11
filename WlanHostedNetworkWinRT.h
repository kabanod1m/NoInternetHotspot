//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#pragma once

#include <windows.devices.wifidirect.h>
#include <wrl\client.h>
#include <wrl\event.h>

#include <iostream>
#include <string>
#include <sstream>
#include <utility>
#include <vector>
#include <map>
#include <stdio.h>
#include <tchar.h>

/// App-specific exception class
class WlanHostedNetworkException : public std::exception
{
public:
    WlanHostedNetworkException(const char* message)
        : std::exception(message),
          _hr(S_OK)
    {}

    WlanHostedNetworkException(const char* message, HRESULT hr)
        : std::exception(message),
        _hr(hr)
    {}

    HRESULT GetErrorCode() const
    {
        return _hr;
    }
private:
    HRESULT _hr;
};

/// Helper interface that can be notified about changes in the "soft AP"
class IWlanHostedNetworkListener
{
public:
    virtual ~IWlanHostedNetworkListener() {}

    virtual void OnDeviceConnected(std::basic_string<TCHAR> deviceId) = 0;
    virtual void OnDeviceDisconnected(std::basic_string<TCHAR> deviceId) = 0;

    virtual void OnAdvertisementStarted() = 0;
    virtual void OnAdvertisementStopped(std::basic_string<TCHAR> message) = 0;
    virtual void OnAdvertisementAborted(std::basic_string<TCHAR> message) = 0;

    virtual void OnAsyncException(std::basic_string<TCHAR> message) = 0;

    virtual void LogMessage(std::basic_string<TCHAR> message) = 0;
};

/// Helper interface to handle user input
class IWlanHostedNetworkPrompt
{
public:
    virtual ~IWlanHostedNetworkPrompt() {};

    virtual bool AcceptIncomingConnection() = 0;
};

/// Wraps code to call into the WiFiDirect WinRT APIs as a replacement for the WlanHostedNetwork* functions
/// https://msdn.microsoft.com/en-us/library/windows.devices.wifidirect.aspx
class WlanHostedNetworkHelper
{
public:
    WlanHostedNetworkHelper();
    ~WlanHostedNetworkHelper();

    /// Set SSID (optional, falls back to a Wi-FI Direct default SSID which begins with "DIRECT-")
    void SetSSID(const std::basic_string<TCHAR>& ssid)
    {
        _ssid = ssid;
        _ssidProvided = true;
    }

    std::basic_string<TCHAR> GetSSID() const
    {
        return _ssid;
    }

    /// Set Passphrase (optional, falls back to a random string)
    void SetPassphrase(const std::basic_string<TCHAR>& passphrase)
    {
        _passphraseProvided = true;
        _passphrase = passphrase;
    }

    std::basic_string<TCHAR> GetPassphrase() const
    {
        return _passphrase;
    }

    /// Register listener to receive updates (only one listener is supported)
    void RegisterListener(IWlanHostedNetworkListener* listener)
    {
        _listener = listener;
    }

    /// Register user prompt to get user input
    void RegisterPrompt(IWlanHostedNetworkPrompt* prompt)
    {
        _prompt = prompt;
    }

    /// Change behavior to auto-accept or ask user
    void SetAutoAccept(bool autoAccept)
    {
        _autoAccept = autoAccept;
    }

    /// Start advertising
    void Start();

    /// Stop advertising
    void Stop();

private:
    /// Start connection listener
    void StartListener();

    /// Clear out old state
    void Reset();

    // WinRT helpers

    /// Main class that is used to start advertisement
    Microsoft::WRL::ComPtr<ABI::Windows::Devices::WiFiDirect::IWiFiDirectAdvertisementPublisher> _publisher;
    /// Advertisement Settings within the publisher
    Microsoft::WRL::ComPtr<ABI::Windows::Devices::WiFiDirect::IWiFiDirectAdvertisement> _advertisement;
    /// Legacy settings within the advertisement settings
    Microsoft::WRL::ComPtr<ABI::Windows::Devices::WiFiDirect::IWiFiDirectLegacySettings> _legacySettings;
    /// Listen for incoming connections
    Microsoft::WRL::ComPtr<ABI::Windows::Devices::WiFiDirect::IWiFiDirectConnectionListener> _connectionListener;

    /// Keep references to all connected peers
    std::map<std::basic_string<TCHAR>, Microsoft::WRL::ComPtr<ABI::Windows::Devices::WiFiDirect::IWiFiDirectDevice>> _connectedDevices;
    std::map<std::basic_string<TCHAR>, EventRegistrationToken> _connectedDeviceStatusChangedTokens;

    /// Used to un-register for events
    EventRegistrationToken _connectionRequestedToken;
    EventRegistrationToken _statusChangedToken;

    // Settings for "soft AP"

    bool _ssidProvided;
    std::basic_string<TCHAR> _ssid;
    bool _passphraseProvided;
    std::basic_string<TCHAR> _passphrase;

    // Listeners that can be notified of changes to the "soft AP"

    IWlanHostedNetworkListener* _listener;

    IWlanHostedNetworkPrompt* _prompt;

    /// tracks whether we should accept incoming connections or ask the user
    bool _autoAccept;
};