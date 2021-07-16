#ifdef _WIN32
#include "../include/BriskAudio.hpp"
#include <atlstr.h>
#include <Audioclient.h>
#include <functiondiscoverykeys_devpkey.h>

#define SAFE_RELEASE(punk)   \
    if ((punk) != nullptr) { \
        (punk)->Release();   \
        (punk) = nullptr;    \
    }

static IMMDeviceEnumerator* spEnumerator = nullptr;
static bool sIsCoInitialized = false;

using namespace BriskAudio;

namespace BriskAudio {
NativeDeviceHandle::NativeDeviceHandle()
{
    pDevice = nullptr;
    pVolume = nullptr;
    eventContext = GUID_NULL;
    pOnVolumeChange = nullptr;
    pOnMute = nullptr;
    pOnDefaultDeviceChange = nullptr;
    pOnDeviceAdd = nullptr;
    pOnDeviceRemove = nullptr;
    cRef_ = 1;
    pDeviceId_ = _wcsdup(L" ");
}

NativeDeviceHandle::~NativeDeviceHandle()
{
    CoTaskMemFree((void*)pDeviceId_);

    SAFE_RELEASE(pVolume)
    SAFE_RELEASE(pDevice)
}

ULONG STDMETHODCALLTYPE NativeDeviceHandle::AddRef()
{
    return InterlockedIncrement(&cRef_);
}

ULONG STDMETHODCALLTYPE NativeDeviceHandle::Release()
{
    ULONG ulRef = InterlockedDecrement(&cRef_);

    if (ulRef == 0) {
        delete this;
    }

    return ulRef;
}

HRESULT STDMETHODCALLTYPE NativeDeviceHandle::QueryInterface(REFIID riid, VOID** ppvInterface)
{
    if (riid == __uuidof(IUnknown)) {
        AddRef();

        // Since both IMMNotificationClient and IAudioEndpointVolumeCallback inherit IUnknown, the compiler needs more info on how to typecast
        *ppvInterface = (IUnknown*)(IMMNotificationClient*)this;
    }
    else if (riid == __uuidof(IMMNotificationClient)) {
        AddRef();

        *ppvInterface = (IMMNotificationClient*)this;
    }
    else if (riid == __uuidof(IAudioEndpointVolumeCallback)) {
        AddRef();

        *ppvInterface = (IAudioEndpointVolumeCallback*)this;
    }
    else {
        *ppvInterface = nullptr;

        return E_NOINTERFACE;
    }

    return S_OK;
}

HRESULT STDMETHODCALLTYPE NativeDeviceHandle::OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDeviceId)
{
    HRESULT status = S_FALSE;
    IMMDevice* pTempDevice = nullptr;
    IPropertyStore* pStore = nullptr;
    PROPVARIANT varName;
    std::string deviceName;

    // This function is called three times (for each device role), so call the callback only once
    if (lstrcmpW(pDeviceId_, pwstrDeviceId) != 0 && pOnDefaultDeviceChange != nullptr) {
        if (FAILED(spEnumerator->GetDevice(pwstrDeviceId, &pTempDevice))) {
            goto Exit;
        }

        if (FAILED(pTempDevice->OpenPropertyStore(STGM_READ, &pStore))) {
            goto Exit;
        }

        if (FAILED(pStore->GetValue(PKEY_Device_FriendlyName, &varName))) {
            goto Exit;
        }

        deviceName = CW2A(varName.pwszVal);

        pOnDefaultDeviceChange(deviceName);

        // Copy current device name for comparison later
        pDeviceId_ = _wcsdup(pwstrDeviceId);

        PropVariantClear(&varName);

        status = S_OK;
    }

Exit:
    SAFE_RELEASE(pStore)
    SAFE_RELEASE(pTempDevice)

    return status;
}

HRESULT STDMETHODCALLTYPE NativeDeviceHandle::OnDeviceAdded(LPCWSTR pwstrDeviceId)
{
    HRESULT status = S_FALSE;
    IMMDevice* pTempDevice = nullptr;
    IPropertyStore* pStore = nullptr;
    PROPVARIANT varName;
    std::string deviceName;

    if (pOnDeviceAdd != nullptr) {
        if (FAILED(spEnumerator->GetDevice(pwstrDeviceId, &pTempDevice))) {
            goto Exit;
        }

        if (FAILED(pTempDevice->OpenPropertyStore(STGM_READ, &pStore))) {
            goto Exit;
        }

        if (FAILED(pStore->GetValue(PKEY_Device_FriendlyName, &varName))) {
            goto Exit;
        }

        deviceName = CW2A(varName.pwszVal);

        pOnDeviceAdd(deviceName);

        PropVariantClear(&varName);

        status = S_OK;
    }

Exit:
    SAFE_RELEASE(pStore)
    SAFE_RELEASE(pTempDevice)

    return status;
};

HRESULT STDMETHODCALLTYPE NativeDeviceHandle::OnDeviceRemoved(LPCWSTR pwstrDeviceId)
{
    HRESULT status = S_FALSE;
    IMMDevice* pTempDevice = nullptr;
    IPropertyStore* pStore = nullptr;
    PROPVARIANT varName;
    std::string deviceName;

    if (pOnDeviceRemove != nullptr) {
        if (FAILED(spEnumerator->GetDevice(pwstrDeviceId, &pTempDevice))) {
            goto Exit;
        }

        if (FAILED(pTempDevice->OpenPropertyStore(STGM_READ, &pStore))) {
            goto Exit;
        }

        if (FAILED(pStore->GetValue(PKEY_Device_FriendlyName, &varName))) {
            goto Exit;
        }

        deviceName = CW2A(varName.pwszVal);

        pOnDeviceRemove(deviceName);

        PropVariantClear(&varName);

        status = S_OK;
    }

Exit:
    SAFE_RELEASE(pStore)
    SAFE_RELEASE(pTempDevice)

    return status;
}

HRESULT STDMETHODCALLTYPE NativeDeviceHandle::OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState)
{
    HRESULT status = S_FALSE;
    IMMDevice* pTempDevice = nullptr;
    IPropertyStore* pStore = nullptr;
    PROPVARIANT varName;
    std::string deviceName;

    if (FAILED(spEnumerator->GetDevice(pwstrDeviceId, &pTempDevice))) {
        goto Exit;
    }

    if (FAILED(pTempDevice->OpenPropertyStore(STGM_READ, &pStore))) {
        goto Exit;
    }

    if (FAILED(pStore->GetValue(PKEY_Device_FriendlyName, &varName))) {
        goto Exit;
    }

    deviceName = CW2A(varName.pwszVal);

    switch (dwNewState) {
    case DEVICE_STATE_DISABLED:
    case DEVICE_STATE_NOTPRESENT:
    case DEVICE_STATE_UNPLUGGED:
        if (pOnDeviceRemove != nullptr) {
            pOnDeviceRemove(deviceName);
        }

        break;

    case DEVICE_STATE_ACTIVE:
        if (pOnDeviceAdd != nullptr) {
            pOnDeviceAdd(deviceName);
        }

        break;
    }

    PropVariantClear(&varName);

    status = S_OK;

Exit:
    SAFE_RELEASE(pStore)
    SAFE_RELEASE(pTempDevice)

    return S_OK;
}

HRESULT STDMETHODCALLTYPE NativeDeviceHandle::OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE NativeDeviceHandle::OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify)
{
    if (pNotify == NULL) {
        return E_INVALIDARG;
    }

    // Call only when volume is changed externally
    if (pNotify->guidEventContext != eventContext) {
        if (pNotify->bMuted == TRUE) {
            if (pOnMute != nullptr) {
                pOnMute();
            }
        }
        else {
            if (pOnVolumeChange != nullptr) {
                pOnVolumeChange(pNotify->fMasterVolume);
            }
        }
    }

    return S_OK;
}

bool Device::isStreamFormatSupported(BufferFormat aFormat, unsigned int aNumChannels, unsigned int aSampleRate)
{
    bool status = false;
    WAVEFORMATEX format;
    IAudioClient* pClient = nullptr;

    if (FAILED(pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void**)&pClient))) {
        goto Exit;
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
        goto Exit;
    }

    status = true;

Exit:
    SAFE_RELEASE(pClient)

    return status;
}

Exit Device::getVolume(float& arVolume)
{
    if (FAILED(pVolume->GetMasterVolumeLevelScalar(&arVolume))) {
        return Exit::FAILURE;
    }

    return Exit::SUCCESS;
}

Exit Device::setVolume(float arVolume)
{
    if (FAILED(pVolume->SetMasterVolumeLevelScalar(arVolume, &eventContext))) {
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
    Exit status = Exit::FAILURE;
    EDataFlow flow = (aType == DeviceType::PLAYBACK) ? eRender : eCapture;
    IMMDeviceCollection* pCollection = nullptr;

    if (FAILED(spEnumerator->EnumAudioEndpoints(flow, DEVICE_STATE_ACTIVE, &pCollection))) {
        goto Exit;
    }

    if (FAILED(pCollection->GetCount(&arDeviceCount))) {
        goto Exit;
    }

    status = Exit::SUCCESS;

Exit:
    SAFE_RELEASE(pCollection)

    return status;
}

Exit openDefaultDevice(Device& arDevice, DeviceType aType)
{
    Exit status = Exit::FAILURE;
    EDataFlow flow = (aType == DeviceType::PLAYBACK) ? eRender : eCapture;
    IPropertyStore* pStore = nullptr;
    PROPVARIANT varName;

    arDevice.type = aType;

    if (FAILED(spEnumerator->GetDefaultAudioEndpoint(flow, eMultimedia, &arDevice.pDevice))) {
        goto Exit;
    }

    if (FAILED(arDevice.pDevice->OpenPropertyStore(STGM_READ, &pStore))) {
        SAFE_RELEASE(arDevice.pDevice)

        goto Exit;
    }

    if (FAILED(pStore->GetValue(PKEY_Device_FriendlyName, &varName))) {
        SAFE_RELEASE(arDevice.pDevice)

        goto Exit;
    }

    arDevice.name = CW2A(varName.pwszVal);

    if (FAILED(arDevice.pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, nullptr, (void**)&arDevice.pVolume))) {
        PropVariantClear(&varName);
        SAFE_RELEASE(arDevice.pDevice)

        goto Exit;
    }

    if (FAILED(spEnumerator->RegisterEndpointNotificationCallback(&arDevice))) {
        SAFE_RELEASE(arDevice.pVolume)
        PropVariantClear(&varName);
        SAFE_RELEASE(arDevice.pDevice)

        goto Exit;
    }

    if (FAILED(arDevice.pVolume->RegisterControlChangeNotify(&arDevice))) {
        SAFE_RELEASE(arDevice.pVolume)
        PropVariantClear(&varName);
        SAFE_RELEASE(arDevice.pDevice)

        goto Exit;
    }

    if (FAILED(CoCreateGuid(&arDevice.eventContext))) {
        SAFE_RELEASE(arDevice.pVolume)
        PropVariantClear(&varName);
        SAFE_RELEASE(arDevice.pDevice)

        goto Exit;
    }

    status = Exit::SUCCESS;

Exit:
    SAFE_RELEASE(pStore)

    return status;
}

Exit openDevice(Device& arDevice, unsigned int aIndex, DeviceType aType)
{
    Exit status = Exit::FAILURE;
    EDataFlow flow = (aType == DeviceType::PLAYBACK) ? eRender : eCapture;
    IMMDeviceCollection* pCollection = nullptr;
    unsigned int count;
    IPropertyStore* pStore = nullptr;
    PROPVARIANT varName;

    arDevice.type = aType;

    if (FAILED(spEnumerator->EnumAudioEndpoints(flow, DEVICE_STATE_ACTIVE, &pCollection))) {
        goto Exit;
    }

    if (FAILED(pCollection->GetCount(&count))) {
        goto Exit;
    }

    if (aIndex >= count) {
        goto Exit;
    }

    if (FAILED(pCollection->Item(aIndex, &arDevice.pDevice))) {
        goto Exit;
    }

    if (FAILED(arDevice.pDevice->OpenPropertyStore(STGM_READ, &pStore))) {
        SAFE_RELEASE(arDevice.pDevice)

        goto Exit;
    }

    if (FAILED(pStore->GetValue(PKEY_Device_FriendlyName, &varName))) {
        SAFE_RELEASE(arDevice.pDevice)

        goto Exit;
    }

    arDevice.name = CW2A(varName.pwszVal);

    if (FAILED(arDevice.pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, nullptr, (void**)&arDevice.pVolume))) {
        PropVariantClear(&varName);
        SAFE_RELEASE(arDevice.pDevice)

        goto Exit;
    }

    status = Exit::SUCCESS;

Exit:
    SAFE_RELEASE(pStore)
    SAFE_RELEASE(pCollection)

    return status;
}

Exit openDevice(Device& arDevice, std::string aDeviceName)
{
    Exit status = Exit::FAILURE;
    IMMDeviceCollection* pCollection = nullptr;
    unsigned int count;
    IMMDevice* pDevice = nullptr;
    IPropertyStore* pStore = nullptr;
    PROPVARIANT varName;
    std::string deviceName;
    IMMEndpoint* pEndpoint = nullptr;
    EDataFlow flow;

    if (FAILED(spEnumerator->EnumAudioEndpoints(eAll, DEVICE_STATE_ACTIVE, &pCollection))) {
        goto Exit;
    }

    if (FAILED(pCollection->GetCount(&count))) {
        goto Exit;
    }

    // Scan the list of available devices
    for (unsigned int i = 0; i < count; i++) {
        if (FAILED(pCollection->Item(i, &pDevice))) {
            goto Exit;
        }

        if (FAILED(pDevice->OpenPropertyStore(STGM_READ, &pStore))) {
            goto Exit;
        }

        if (FAILED(pStore->GetValue(PKEY_Device_FriendlyName, &varName))) {
            goto Exit;
        }

        deviceName = CW2A(varName.pwszVal);

        // If device is found
        if (aDeviceName == deviceName) {
            if (FAILED(pDevice->QueryInterface(__uuidof(IMMEndpoint), (void**)&pEndpoint))) {
                PropVariantClear(&varName);

                goto Exit;
            }

            if (FAILED(pEndpoint->GetDataFlow(&flow))) {
                PropVariantClear(&varName);

                goto Exit;
            }

            if (FAILED(arDevice.pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, nullptr, (void**)&arDevice.pVolume))) {
                PropVariantClear(&varName);

                goto Exit;
            }

            arDevice.name = deviceName;
            arDevice.type = (flow == eRender) ? DeviceType::PLAYBACK : DeviceType::CAPTURE;
            arDevice.pDevice = pDevice;

            status = Exit::SUCCESS;

            break;
        }

        // Cleanup for reuse
        PropVariantClear(&varName);
        pStore->Release();
        pDevice->Release();
    }

Exit:
    SAFE_RELEASE(pEndpoint)
    SAFE_RELEASE(pStore)
    SAFE_RELEASE(pDevice)
    SAFE_RELEASE(pCollection)

    return status;
}

Exit closeDevice(Device& arDevice)
{
    Exit status = Exit::FAILURE;

    if (arDevice.pDevice == nullptr || arDevice.pVolume == nullptr) {
        return Exit::FAILURE;
    }

    if (FAILED(arDevice.pVolume->UnregisterControlChangeNotify(&arDevice))) {
        goto Exit;
    }

    if (FAILED(spEnumerator->UnregisterEndpointNotificationCallback(&arDevice))) {
        goto Exit;
    }

    status = Exit::SUCCESS;

Exit:
    SAFE_RELEASE(arDevice.pVolume)
    SAFE_RELEASE(arDevice.pDevice)

    return status;
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
