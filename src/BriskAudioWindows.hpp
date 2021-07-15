#pragma once

#ifdef _WIN32
#include <Endpointvolume.h>
#include <mmdeviceapi.h>

namespace BriskAudio {
struct NativeDeviceHandle : public IAudioEndpointVolumeCallback {
    IMMDevice* pDevice;
    IAudioEndpointVolume* pVolume;
    GUID eventContext;
    LONG cRef_;
    void (*pOnVolumeChange)(float aVolume);
    void (*pOnMute)();

    NativeDeviceHandle();
    virtual ~NativeDeviceHandle();
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, VOID** ppvInterface);
    HRESULT STDMETHODCALLTYPE OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify);
};
}
#endif