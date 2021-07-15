#pragma once

#include <string>

#ifdef _WIN32
#include <Endpointvolume.h>
#include <mmdeviceapi.h>
#endif

namespace BriskAudio {
enum class Exit {
    SUCCESS,
    FAILURE
};

enum class DeviceType {
    PLAYBACK,
    CAPTURE
};

enum class BufferFormat {
    U_INT_8,
    S_INT_16,
    S_INT_24,
    S_INT_32,
    FLOAT_32,
    FLOAT_64
};

struct Stream {
    void* nativeHandle;

    Stream()
    {
        nativeHandle = nullptr;
        format_ = BufferFormat::U_INT_8;
        numChannels_ = 0;
        sampleRate_ = 0;
    }

private:
    BufferFormat format_;
    unsigned int numChannels_;
    unsigned int sampleRate_;
};

#ifdef _WIN32
struct NativeDeviceHandle {
    IMMDevice* pDevice;
    IAudioEndpointVolume* pVolume;

    NativeDeviceHandle();
    ~NativeDeviceHandle();
};

struct DeviceEventNotifier : public IMMNotificationClient, public IAudioEndpointVolumeCallback {
    DeviceEventNotifier();
    virtual ~DeviceEventNotifier();
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, VOID **ppvInterface);
    HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDeviceId);
    HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR pwstrDeviceId);
    HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR pwstrDeviceId);
    HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState);
    HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key);
    HRESULT STDMETHODCALLTYPE OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify);
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
#endif

struct Device {
    std::string name;
    DeviceType type;
    NativeDeviceHandle handle;

    Device()
    {
        name = "??????";
        type = DeviceType::PLAYBACK;
    }

    Exit getVolume(float& arVolume);
    Exit setVolume(float aVolume);
    bool isStreamFormatSupported(BufferFormat aFormat, unsigned int aNumChannels, unsigned int aSampleRate);
    Exit openStream(Stream& arStream, BufferFormat aFormat, unsigned int aNumChannels, unsigned int aSampleRate);
    Exit closeStream(Stream& arStream);
};

Exit init();
Exit getDeviceCount(unsigned int& arDeviceCount, DeviceType aType);
Exit openDefaultDevice(Device& arDevice, DeviceType aType);
Exit openDevice(Device& arDevice, unsigned int aIndex, DeviceType aType);
Exit openDevice(Device& arDevice, std::string aDeviceName);
Exit closeDevice(Device& arDevice);
Exit quit();
}