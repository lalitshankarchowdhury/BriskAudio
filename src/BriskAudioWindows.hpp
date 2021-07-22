#pragma once

#ifdef _WIN32
#include <Audioclient.h>
#include <Endpointvolume.h>
#include <atlbase.h>
#include <initguid.h>
#include <mmdeviceapi.h>
#include <string>

namespace BriskAudio {
struct NativeStreamHandle {
    CComPtr<IAudioRenderClient> pRenderClient;
    CComPtr<IAudioCaptureClient> pCaptureClient;

    NativeStreamHandle();
};

struct NativeDeviceHandle : public IAudioEndpointVolumeCallback, public IMMNotificationClient {
    CComPtr<IMMDevice> pDevice;
    CComPtr<IAudioClient> pClient;
    CComPtr<IAudioEndpointVolume> pVolume;
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
    LONG referenceCount_;
    LPCWSTR pDeviceId_;
};
}
#endif
