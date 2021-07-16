#pragma once

#ifdef _WIN32
#include <string>
#include <Endpointvolume.h>
#include <mmdeviceapi.h>

namespace BriskAudio {
struct NativeDeviceHandle : public IAudioEndpointVolumeCallback, public IMMNotificationClient {
    IMMDevice* pDevice;
    IAudioEndpointVolume* pVolume;
    GUID eventContext;
    void (*pOnVolumeChange)(float aVolume);
    void (*pOnMute)();
    void (*pOnDefaultDeviceChange)(std::string aDeviceName);
    void (*pOnDeviceAdd)(std::string aDeviceName);
    void (*pOnDeviceRemove)(std::string aDeviceName);

    NativeDeviceHandle();
    virtual ~NativeDeviceHandle();
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, VOID** ppvInterface);
    HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDeviceId);
    HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR pwstrDeviceId);
    HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR pwstrDeviceId);
    HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState);
    HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key);
    HRESULT STDMETHODCALLTYPE OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify);

private:
    LONG cRef_;
    LPCWSTR pDeviceId_;
};
}
#endif
