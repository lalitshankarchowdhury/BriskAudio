#ifdef _WIN32
#include "BriskAudio.hpp"
#include <atlstr.h>
#include <Audioclient.h>
#include <functiondiscoverykeys_devpkey.h>

static IMMDeviceEnumerator* spEnumerator = nullptr;
static bool sIsCoInitialized = false;

using namespace BriskAudio;

namespace BriskAudio {
NativeDeviceHandle::NativeDeviceHandle()
{
    pDevice = nullptr;
    pVolume = nullptr;
}

NativeDeviceHandle::~NativeDeviceHandle()
{
    if (pVolume != nullptr) {
        pVolume->Release();
        pVolume = nullptr;
    }

    if (pDevice != nullptr) {
        pDevice->Release();
        pDevice = nullptr;
    }
}

bool Device::isStreamFormatSupported(BufferFormat aFormat, unsigned int aNumChannels, unsigned int aSampleRate)
{
    WAVEFORMATEX format;
    IAudioClient* pClient = nullptr;

    if (FAILED(handle.pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void**)&pClient))) {
        return false;
    }

    format.wFormatTag = (WORD)((aFormat == BufferFormat::FLOAT_32 || aFormat == BufferFormat::FLOAT_64) ? WAVE_FORMAT_IEEE_FLOAT : WAVE_FORMAT_PCM);
    format.nChannels = (WORD)aNumChannels;
    format.nSamplesPerSec = aSampleRate;

    if (aFormat == BufferFormat::U_INT_8) {
        format.wBitsPerSample = 8;
    }
    else if (aFormat == BufferFormat::S_INT_16) {
        format.wBitsPerSample = 16;
    }
    else if (aFormat == BufferFormat::S_INT_24) {
        format.wBitsPerSample = 24;
    }
    else if (aFormat == BufferFormat::S_INT_32) {
        format.wBitsPerSample = 32;
    }
    else if (aFormat == BufferFormat::FLOAT_32) {
        format.wBitsPerSample = 32;
    }
    else {
        format.wBitsPerSample = 64;
    }

    format.nBlockAlign = format.nChannels * (format.wBitsPerSample / 8);
    format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
    format.cbSize = 22;

    if (FAILED(pClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, &format, nullptr))) {
        pClient->Release();

        return false;
    }

    pClient->Release();

    return true;
}

Exit Device::getVolume(float& arVolume)
{
    if (FAILED(handle.pVolume->GetMasterVolumeLevelScalar(&arVolume))) {
        return Exit::FAILURE;
    }

    return Exit::SUCCESS;
}

Exit Device::setVolume(float arVolume)
{
    if (FAILED(handle.pVolume->SetMasterVolumeLevelScalar(arVolume, nullptr))) {
        return Exit::FAILURE;
    }

    return Exit::FAILURE;
}

Exit Device::openStream(Stream& arStream, BufferFormat aFormat, unsigned int aNumChannels, unsigned int aSampleRate)
{
    return Exit::FAILURE;
}

Exit Device::closeStream(Stream& arStream)
{
    return Exit::FAILURE;
}

DeviceEventNotifier::DeviceEventNotifier()
{
    cRef_ = 1;
    pDeviceId_ = _wcsdup(L" ");
    pOnDefaultDeviceChange_ = nullptr;
    pOnDeviceAdd_ = nullptr;
    pOnDeviceRemove_ = nullptr;
}

DeviceEventNotifier::~DeviceEventNotifier()
{
    CoTaskMemFree((void*)pDeviceId_);
}

ULONG STDMETHODCALLTYPE DeviceEventNotifier::AddRef()
{
    return InterlockedIncrement(&cRef_);
}

ULONG STDMETHODCALLTYPE DeviceEventNotifier::Release()
{
    ULONG ulRef = InterlockedDecrement(&cRef_);
    
    if (ulRef == 0)
    {
        delete this;
    }

    return ulRef;
}

HRESULT STDMETHODCALLTYPE DeviceEventNotifier::QueryInterface(REFIID riid, VOID **ppvInterface)
{
    if (riid == __uuidof(IUnknown))
    {
        AddRef();

        // Since both IMMNotificationClient and IAudioEndpointVolumeCallback inherit IUnknown, the compiler needs more info on how to typecast
        *ppvInterface = (IUnknown*)(IMMNotificationClient*)this;
    }
    else if (riid == __uuidof(IMMNotificationClient))
    {
        AddRef();

        *ppvInterface = (IMMNotificationClient*)this;
    }
    else if (riid == __uuidof(IAudioEndpointVolumeCallback))
    {
        AddRef();

        *ppvInterface = (IAudioEndpointVolumeCallback*)this;
    }
    else
    {
        *ppvInterface = nullptr;

        return E_NOINTERFACE;
    }

    return S_OK;
}

HRESULT STDMETHODCALLTYPE DeviceEventNotifier::OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDeviceId)
{
    IMMDevice* pDevice = nullptr;
    IPropertyStore* pStore = nullptr;
    PROPVARIANT varName;
    DeviceType type;

    // This function is called three times (for each device role), so call the callback only once
    if (lstrcmpW(pDeviceId_, pwstrDeviceId) != 0 && pOnDefaultDeviceChange_ != nullptr) {
        if (FAILED(spEnumerator->GetDevice(pwstrDeviceId, &pDevice))) {
            return S_FALSE;
        }

        if (FAILED(pDevice->OpenPropertyStore(STGM_READ, &pStore))) {
            pDevice->Release();

            return S_FALSE;
        }

        if (FAILED(pStore->GetValue(PKEY_Device_FriendlyName, &varName))) {
            pStore->Release();
            pDevice->Release();

            return S_FALSE;
        }

        type = (flow == eRender) ? DeviceType::PLAYBACK : DeviceType::CAPTURE;

        pOnDefaultDeviceChange_(std::string(CW2A(varName.pwszVal)), type);

        pDeviceId_ = _wcsdup(pwstrDeviceId);

        if (FAILED(PropVariantClear(&varName))) {
            pStore->Release();
            pDevice->Release();

            return S_FALSE;
        }

        pStore->Release();
        pDevice->Release();
    }

    return S_OK;
}

HRESULT STDMETHODCALLTYPE DeviceEventNotifier::OnDeviceAdded(LPCWSTR pwstrDeviceId)
{
    IMMDevice* pDevice = nullptr;
    IPropertyStore* pStore = nullptr;
    PROPVARIANT varName;
    std::string deviceName;
    IMMEndpoint* pEndpoint = nullptr;
    EDataFlow flow;
    DeviceType type;

    if (pOnDeviceAdd_ != nullptr) {
        if (FAILED(spEnumerator->GetDevice(pwstrDeviceId, &pDevice))) {
            return S_FALSE;
        }

        if (FAILED(pDevice->OpenPropertyStore(STGM_READ, &pStore))) {
            pDevice->Release();

            return S_FALSE;
        }

        if (FAILED(pStore->GetValue(PKEY_Device_FriendlyName, &varName))) {
            pStore->Release();
            pDevice->Release();

            return S_FALSE;
        }

        deviceName = CW2A(varName.pwszVal);

        if (FAILED(pDevice->QueryInterface(__uuidof(IMMEndpoint), (void**)&pEndpoint))) {
            PropVariantClear(&varName);

            pStore->Release();
            pDevice->Release();

            return S_FALSE;
        }

        if (FAILED(pEndpoint->GetDataFlow(&flow))) {
            pEndpoint->Release();

            PropVariantClear(&varName);

            pStore->Release();
            pDevice->Release();

            return S_FALSE;
        }

        type = (flow == eRender) ? DeviceType::PLAYBACK : DeviceType::CAPTURE;

        pOnDeviceAdd_(deviceName, type);

        pEndpoint->Release();

        if (FAILED(PropVariantClear(&varName))) {
            pStore->Release();
            pDevice->Release();

            return S_FALSE;
        }

        pStore->Release();
        pDevice->Release();
    }

    return S_OK;
};

HRESULT STDMETHODCALLTYPE DeviceEventNotifier::OnDeviceRemoved(LPCWSTR pwstrDeviceId)
{
    IMMDevice* pDevice = nullptr;
    IPropertyStore* pStore = nullptr;
    PROPVARIANT varName;
    std::string deviceName;
    IMMEndpoint* pEndpoint = nullptr;
    EDataFlow flow;
    DeviceType type;

    if (pOnDeviceRemove_ != nullptr) {
        if (FAILED(spEnumerator->GetDevice(pwstrDeviceId, &pDevice))) {
            return S_FALSE;
        }

        if (FAILED(pDevice->OpenPropertyStore(STGM_READ, &pStore))) {
            pDevice->Release();

            return S_FALSE;
        }

        if (FAILED(pStore->GetValue(PKEY_Device_FriendlyName, &varName))) {
            pStore->Release();
            pDevice->Release();

            return S_FALSE;
        }

        deviceName = CW2A(varName.pwszVal);

        if (FAILED(pDevice->QueryInterface(__uuidof(IMMEndpoint), (void**)&pEndpoint))) {
            PropVariantClear(&varName);

            pStore->Release();
            pDevice->Release();

            return S_FALSE;
        }

        if (FAILED(pEndpoint->GetDataFlow(&flow))) {
            pEndpoint->Release();

            PropVariantClear(&varName);

            pStore->Release();
            pDevice->Release();

            return S_FALSE;
        }

        type = (flow == eRender) ? DeviceType::PLAYBACK : DeviceType::CAPTURE;

        pOnDeviceRemove_(deviceName, type);

        pEndpoint->Release();

        if (FAILED(PropVariantClear(&varName))) {
            pStore->Release();
            pDevice->Release();

            return S_FALSE;
        }

        pStore->Release();
        pDevice->Release();
    }

    return S_OK;
}

HRESULT STDMETHODCALLTYPE DeviceEventNotifier::OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState)
{
    IMMDevice* pDevice = nullptr;
    IPropertyStore* pStore = nullptr;
    PROPVARIANT varName;
    std::string deviceName;
    IMMEndpoint* pEndpoint = nullptr;
    EDataFlow flow;
    DeviceType type;

    if (FAILED(spEnumerator->GetDevice(pwstrDeviceId, &pDevice))) {
        return S_FALSE;
    }

    if (FAILED(pDevice->OpenPropertyStore(STGM_READ, &pStore))) {
        pDevice->Release();

        return S_FALSE;
    }

    if (FAILED(pStore->GetValue(PKEY_Device_FriendlyName, &varName))) {
        pStore->Release();
        pDevice->Release();

        return S_FALSE;
    }

    deviceName = CW2A(varName.pwszVal);

    if (FAILED(pDevice->QueryInterface(__uuidof(IMMEndpoint), (void**)&pEndpoint))) {
        PropVariantClear(&varName);

        pStore->Release();
        pDevice->Release();

        return S_FALSE;
    }

    if (FAILED(pEndpoint->GetDataFlow(&flow))) {
        pEndpoint->Release();

        PropVariantClear(&varName);

        pStore->Release();
        pDevice->Release();

        return S_FALSE;
    }

    type = (flow == eRender) ? DeviceType::PLAYBACK : DeviceType::CAPTURE;

    switch (dwNewState) {
    case DEVICE_STATE_DISABLED:
    case DEVICE_STATE_NOTPRESENT:
    case DEVICE_STATE_UNPLUGGED:
        if (pOnDeviceRemove_ != nullptr) {
            pOnDeviceRemove_(deviceName, type);
        }

        break;

    case DEVICE_STATE_ACTIVE:
        if (pOnDeviceAdd_ != nullptr) {
            pOnDeviceAdd_(deviceName, type);
        }

        break;
    }

    pEndpoint->Release();

    if (FAILED(PropVariantClear(&varName))) {
        pStore->Release();
        pDevice->Release();

        return S_FALSE;
    }

    pStore->Release();
    pDevice->Release();

    return S_OK;
}

HRESULT STDMETHODCALLTYPE DeviceEventNotifier::OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE DeviceEventNotifier::OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify)
{
    return S_FALSE;
}

Exit DeviceEventNotifier::registerEventCallbacks(
    void (*apOnDefaultDeviceChange)(std::string aDeviceName, DeviceType aType),
    void (*apOnDeviceAdd)(std::string aDeviceName, DeviceType aType),
    void (*apOnDeviceRemove)(std::string aDeviceName, DeviceType aType))
{
    pOnDefaultDeviceChange_ = apOnDefaultDeviceChange;
    pOnDeviceAdd_ = apOnDeviceAdd;
    pOnDeviceRemove_ = apOnDeviceRemove;

    if (FAILED(spEnumerator->RegisterEndpointNotificationCallback(this))) {
        return Exit::FAILURE;
    }

    return Exit::SUCCESS;
}

Exit DeviceEventNotifier::unregisterEventCallbacks()
{
    pOnDefaultDeviceChange_ = nullptr;
    pOnDeviceAdd_ = nullptr;
    pOnDeviceRemove_ = nullptr;

    if (FAILED(spEnumerator->UnregisterEndpointNotificationCallback(this))) {
        return Exit::FAILURE;
    }

    return Exit::SUCCESS;
}

Exit init()
{
    // If BriskAudio is already initialized
    if (spEnumerator != nullptr) {
        return Exit::FAILURE;
    }

    // CoInitialize() must be called only once
    if (!sIsCoInitialized) {
        if (FAILED(CoInitialize(nullptr))) {

            return Exit::FAILURE;
        }

        sIsCoInitialized = true;
    }

    if (FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&spEnumerator))) {
        CoUninitialize();

        return Exit::FAILURE;
    }

    return Exit::SUCCESS;
}

Exit getDeviceCount(unsigned int& arDeviceCount, DeviceType aType)
{
    EDataFlow flow = (aType == DeviceType::PLAYBACK) ? eRender : eCapture;
    IMMDeviceCollection* pCollection = nullptr;

    if (FAILED(spEnumerator->EnumAudioEndpoints(flow, DEVICE_STATE_ACTIVE, &pCollection))) {
        return Exit::FAILURE;
    }

    if (FAILED(pCollection->GetCount(&arDeviceCount))) {
        pCollection->Release();

        return Exit::FAILURE;
    }

    pCollection->Release();

    return Exit::SUCCESS;
}

Exit openDefaultDevice(Device& arDevice, DeviceType aType)
{
    EDataFlow flow = (aType == DeviceType::PLAYBACK) ? eRender : eCapture;
    IPropertyStore* pStore = nullptr;
    PROPVARIANT varName;

    arDevice.type = aType;

    if (FAILED(spEnumerator->GetDefaultAudioEndpoint(flow, eMultimedia, &arDevice.handle.pDevice))) {
        return Exit::FAILURE;
    }

    if (FAILED(arDevice.handle.pDevice->OpenPropertyStore(STGM_READ, &pStore))) {
        arDevice.handle.pDevice->Release();

        return Exit::FAILURE;
    }

    if (FAILED(pStore->GetValue(PKEY_Device_FriendlyName, &varName))) {
        pStore->Release();
        arDevice.handle.pDevice->Release();

        return Exit::FAILURE;
    }

    arDevice.name = CW2A(varName.pwszVal);

    if (FAILED(arDevice.handle.pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, nullptr, (void**)&arDevice.handle.pVolume))) {
        PropVariantClear(&varName);
        pStore->Release();
        arDevice.handle.pDevice->Release();
        return Exit::FAILURE;
    }

    PropVariantClear(&varName);
    pStore->Release();

    return Exit::SUCCESS;
}

Exit openDevice(Device& arDevice, unsigned int aIndex, DeviceType aType)
{
    EDataFlow flow = (aType == DeviceType::PLAYBACK) ? eRender : eCapture;
    IMMDeviceCollection* pCollection = nullptr;
    unsigned int count;
    IPropertyStore* pStore = nullptr;
    PROPVARIANT varName;

    arDevice.type = aType;

    if (FAILED(spEnumerator->EnumAudioEndpoints(flow, DEVICE_STATE_ACTIVE, &pCollection))) {
        return Exit::FAILURE;
    }

    if (FAILED(pCollection->GetCount(&count))) {
        pCollection->Release();

        return Exit::FAILURE;
    }

    if (aIndex >= count) {
        pCollection->Release();

        return Exit::FAILURE;
    }

    if (FAILED(pCollection->Item(aIndex, &arDevice.handle.pDevice))) {
        pCollection->Release();

        return Exit::FAILURE;
    }

    if (FAILED(arDevice.handle.pDevice->OpenPropertyStore(STGM_READ, &pStore))) {
        arDevice.handle.pDevice->Release();
        pCollection->Release();

        return Exit::FAILURE;
    }

    if (FAILED(pStore->GetValue(PKEY_Device_FriendlyName, &varName))) {
        pStore->Release();
        arDevice.handle.pDevice->Release();
        pCollection->Release();

        return Exit::FAILURE;
    }

    arDevice.name = CW2A(varName.pwszVal);

    if (FAILED(arDevice.handle.pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, nullptr, (void**)&arDevice.handle.pVolume))) {
        PropVariantClear(&varName);
        pStore->Release();
        arDevice.handle.pDevice->Release();
        pCollection->Release();

        return Exit::FAILURE;
    }

    PropVariantClear(&varName);
    pStore->Release();
    pCollection->Release();

    return Exit::SUCCESS;
}

Exit openDevice(Device& arDevice, std::string aDeviceName)
{
    IMMDeviceCollection* pCollection = nullptr;
    unsigned int count;
    IMMDevice* pDevice = nullptr;
    IPropertyStore* pStore = nullptr;
    PROPVARIANT varName;
    std::string deviceName;
    IMMEndpoint* pEndpoint = nullptr;
    EDataFlow flow;

    if (FAILED(spEnumerator->EnumAudioEndpoints(eAll, DEVICE_STATE_ACTIVE, &pCollection))) {
        return Exit::FAILURE;
    }

    if (FAILED(pCollection->GetCount(&count))) {
        pCollection->Release();

        return Exit::FAILURE;
    }

    // Scan the list of available devices
    for (unsigned int i = 0; i < count; i++) {
        if (FAILED(pCollection->Item(i, &pDevice))) {
            pCollection->Release();

            return Exit::FAILURE;
        }

        if (FAILED(pDevice->OpenPropertyStore(STGM_READ, &pStore))) {
            pDevice->Release();
            pCollection->Release();

            return Exit::FAILURE;
        }

        if (FAILED(pStore->GetValue(PKEY_Device_FriendlyName, &varName))) {
            pStore->Release();
            pDevice->Release();
            pCollection->Release();

            return Exit::FAILURE;
        }

        deviceName = CW2A(varName.pwszVal);

        // If device is found
        if (aDeviceName == deviceName) {
            if (FAILED(pDevice->QueryInterface(__uuidof(IMMEndpoint), (void**)&pEndpoint))) {
                PropVariantClear(&varName);
                pStore->Release();
                pDevice->Release();
                pCollection->Release();

                return Exit::FAILURE;
            }

            if (FAILED(pEndpoint->GetDataFlow(&flow))) {
                pEndpoint->Release();
                PropVariantClear(&varName);
                pStore->Release();
                pDevice->Release();
                pCollection->Release();

                return Exit::FAILURE;
            }

            if (FAILED(arDevice.handle.pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, nullptr, (void**)&arDevice.handle.pVolume))) {
                pEndpoint->Release();
                PropVariantClear(&varName);
                pStore->Release();
                pDevice->Release();
                pCollection->Release();

                return Exit::FAILURE;
            }

            arDevice.name = deviceName;
            arDevice.type = (flow == eRender) ? DeviceType::PLAYBACK : DeviceType::CAPTURE;
            arDevice.handle.pDevice = pDevice;

            pEndpoint->Release();
            PropVariantClear(&varName);
            pStore->Release();
            pCollection->Release();

            return Exit::SUCCESS;
        }

        PropVariantClear(&varName);
        pStore->Release();
        pDevice->Release();
    }

    PropVariantClear(&varName);
    pStore->Release();
    pDevice->Release();
    pCollection->Release();

    return Exit::FAILURE;
}

Exit closeDevice(Device& arDevice)
{
    if (arDevice.handle.pDevice == nullptr) {
        return Exit::FAILURE;
    }

    arDevice.handle.pDevice->Release();
    arDevice.handle.pDevice = nullptr;

    return Exit::SUCCESS;
}

Exit quit()
{
    // If BriskAudio is already uninitialized/not initialized
    if (spEnumerator == nullptr) {
        return Exit::FAILURE;
    }

    spEnumerator->Release();
    spEnumerator = nullptr;

    // CoUninitialize() must be called only once when the program exits
    atexit((void(__cdecl*)())CoUninitialize);

    return Exit::SUCCESS;
}
}
#endif