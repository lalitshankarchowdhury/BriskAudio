#pragma once

#ifdef _WIN32
#include "../include/BriskAudio.hpp"
#include <mmdeviceapi.h>

namespace BriskAudio {
struct DeviceEventNotifier : IMMNotificationClient {
    DeviceEventNotifier();
    virtual ~DeviceEventNotifier();
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, VOID** ppvInterface);
    HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDeviceId);
    HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR pwstrDeviceId);
    HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR pwstrDeviceId);
    HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState);
    HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key);
    Exit registerEventCallbacks(
        void (*apOnDefaultDeviceChange)(std::string aDeviceName, DeviceType aType),
        void (*apOnDeviceAdd)(std::string aDeviceName, DeviceType aType),
        void (*apOnDeviceRemove)(std::string aDeviceName, DeviceType aType));
    Exit unregisterEventCallbacks();

private:
    LONG cRef_;
    LPCWSTR pDeviceId_;
    void (*pOnDefaultDeviceChange_)(std::string aDeviceName, DeviceType aType);
    void (*pOnDeviceAdd_)(std::string aDeviceName, DeviceType aType);
    void (*pOnDeviceRemove_)(std::string aDeviceName, DeviceType aType);
};
}
#endif
