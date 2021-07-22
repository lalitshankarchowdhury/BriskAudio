#ifdef _WIN32
#include "../include/BriskAudio.hpp"
#include <array>
#include <functiondiscoverykeys_devpkey.h>

static CComPtr<IMMDeviceEnumerator> spEnumerator = nullptr;
static bool sIsCoInitialized = false;

using namespace BriskAudio;

namespace BriskAudio {
NativeStreamHandle::NativeStreamHandle()
{
    pRenderClient = nullptr;
    pCaptureClient = nullptr;
}

NativeDeviceHandle::NativeDeviceHandle()
{
    pDevice = nullptr;
    pClient = nullptr;
    pVolume = nullptr;
    eventContext = GUID_NULL;
    pOnVolumeChange = nullptr;
    pOnMute = nullptr;
    pOnDefaultDeviceChange = nullptr;
    pOnDeviceAdd = nullptr;
    pOnDeviceRemove = nullptr;
    referenceCount_ = 1;
    pDeviceId_ = L" ";
}

NativeDeviceHandle::~NativeDeviceHandle() { }

ULONG STDMETHODCALLTYPE NativeDeviceHandle::AddRef()
{
    return InterlockedIncrement(&referenceCount_);
}

ULONG STDMETHODCALLTYPE NativeDeviceHandle::Release()
{
    ULONG referenceCount = InterlockedDecrement(&referenceCount_);

    if (referenceCount == 0) {
        delete this;
    }

    return referenceCount;
}

HRESULT STDMETHODCALLTYPE NativeDeviceHandle::QueryInterface(REFIID riid, VOID** ppvInterface)
{
    if (riid == __uuidof(IUnknown)) {
        AddRef();

        // Since both IMMNotificationClient and IAudioEndpointVolumeCallback inherit IUnknown, the compiler needs more info on how to typecast
        *ppvInterface = static_cast<IUnknown*>(static_cast<IMMNotificationClient*>(this));
    } else if (riid == __uuidof(IMMNotificationClient)) {
        AddRef();

        *ppvInterface = static_cast<IMMNotificationClient*>(this);
    } else if (riid == __uuidof(IAudioEndpointVolumeCallback)) {
        AddRef();

        *ppvInterface = static_cast<IAudioEndpointVolumeCallback*>(this);
    } else {
        *ppvInterface = nullptr;

        return E_NOINTERFACE;
    }

    return S_OK;
}

HRESULT STDMETHODCALLTYPE NativeDeviceHandle::OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDeviceId)
{
    CComPtr<IMMDevice> pTempDevice = nullptr;
    CComPtr<IPropertyStore> pStore = nullptr;
    PROPVARIANT variant;
    std::string deviceName;

    // This function is called three times (for each device role change), so call the callback only once
    if (lstrcmpW(pDeviceId_, pwstrDeviceId) != 0 && pOnDefaultDeviceChange != nullptr) {
        if (FAILED(spEnumerator->GetDevice(pwstrDeviceId, &pTempDevice))) {
            return S_FALSE;
        }

        if (FAILED(pTempDevice->OpenPropertyStore(STGM_READ, &pStore))) {
            return S_FALSE;
        }

        if (FAILED(pStore->GetValue(PKEY_Device_FriendlyName, &variant))) {
            return S_FALSE;
        }

        deviceName = CW2A(variant.pwszVal);

        pOnDefaultDeviceChange(deviceName);

        // Copy current device name for comparison later
        pDeviceId_ = pwstrDeviceId;
    }

    PropVariantClear(&variant);

    return S_OK;
}

HRESULT STDMETHODCALLTYPE NativeDeviceHandle::OnDeviceAdded(LPCWSTR pwstrDeviceId)
{
    CComPtr<IMMDevice> pTempDevice = nullptr;
    CComPtr<IPropertyStore> pStore = nullptr;
    PROPVARIANT variant;
    std::string deviceName;

    if (pOnDeviceAdd != nullptr) {
        if (FAILED(spEnumerator->GetDevice(pwstrDeviceId, &pTempDevice))) {
            return S_FALSE;
        }

        if (FAILED(pTempDevice->OpenPropertyStore(STGM_READ, &pStore))) {
            return S_FALSE;
        }

        if (FAILED(pStore->GetValue(PKEY_Device_FriendlyName, &variant))) {
            return S_FALSE;
        }

        deviceName = CW2A(variant.pwszVal);

        pOnDeviceAdd(deviceName);
    }

    PropVariantClear(&variant);

    return S_OK;
};

HRESULT STDMETHODCALLTYPE NativeDeviceHandle::OnDeviceRemoved(LPCWSTR pwstrDeviceId)
{
    CComPtr<IMMDevice> pTempDevice = nullptr;
    CComPtr<IPropertyStore> pStore = nullptr;
    PROPVARIANT variant;
    std::string deviceName;

    if (pOnDeviceRemove != nullptr) {
        if (FAILED(spEnumerator->GetDevice(pwstrDeviceId, &pTempDevice))) {
            return S_FALSE;
        }

        if (FAILED(pTempDevice->OpenPropertyStore(STGM_READ, &pStore))) {
            return S_FALSE;
        }

        if (FAILED(pStore->GetValue(PKEY_Device_FriendlyName, &variant))) {
            return S_FALSE;
        }

        deviceName = CW2A(variant.pwszVal);

        pOnDeviceRemove(deviceName);
    }

    PropVariantClear(&variant);

    return S_OK;
}

HRESULT STDMETHODCALLTYPE NativeDeviceHandle::OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState)
{
    CComPtr<IMMDevice> pTempDevice = nullptr;
    CComPtr<IPropertyStore> pStore = nullptr;
    PROPVARIANT variant;
    std::string deviceName;

    if (FAILED(spEnumerator->GetDevice(pwstrDeviceId, &pTempDevice))) {
        return S_FALSE;
    }

    if (FAILED(pTempDevice->OpenPropertyStore(STGM_READ, &pStore))) {
        return S_FALSE;
    }

    if (FAILED(pStore->GetValue(PKEY_Device_FriendlyName, &variant))) {
        return S_FALSE;
    }

    deviceName = CW2A(variant.pwszVal);

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

    PropVariantClear(&variant);

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
        } else {
            if (pOnVolumeChange != nullptr) {
                pOnVolumeChange(pNotify->fMasterVolume);
            }
        }
    }

    return S_OK;
}

Stream::Stream()
{
    numChannels_ = 0;
    sampleRate_ = 0;
    format_ = static_cast<BufferFormat>(0);
    latency_ = 0.0f;
}

Device::Device()
{
    name = "??????";
    type = static_cast<DeviceType>(0);
    supportedFormats = static_cast<BufferFormat>(0);
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

bool Device::isStreamFormatSupported(unsigned int aNumChannels, unsigned int aSampleRate, BufferFormat aFormat)
{
    WAVEFORMATEX format;

    format.nChannels = static_cast<WORD>(aNumChannels);
    format.nSamplesPerSec = aSampleRate;
    format.wFormatTag = static_cast<WORD>(((aFormat == BufferFormat::FLOAT_32 || aFormat == BufferFormat::FLOAT_64) ? WAVE_FORMAT_IEEE_FLOAT : WAVE_FORMAT_PCM));

    if (aFormat == BufferFormat::U_INT_8) {
        format.wBitsPerSample = 8;
    } else if (aFormat == BufferFormat::S_INT_16) {
        format.wBitsPerSample = 16;
    } else if (aFormat == BufferFormat::S_INT_24) {
        format.wBitsPerSample = 24;
    } else if (aFormat == BufferFormat::S_INT_32) {
        format.wBitsPerSample = 32;
    } else if (aFormat == BufferFormat::FLOAT_32) {
        format.wBitsPerSample = 32;
    } else {
        format.wBitsPerSample = 64;
    }

    format.nBlockAlign = format.nChannels * (format.wBitsPerSample / 8);
    format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
    format.cbSize = 0;

    if (FAILED(pClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, &format, nullptr))) {
        return false;
    }

    return true;
}

Exit Device::openStream(Stream& aStream, unsigned int aNumChannels, unsigned int aSampleRate, BufferFormat aFormat, float aLatency)
{
    REFERENCE_TIME latency = static_cast<REFERENCE_TIME>(aLatency * 10000);
    WAVEFORMATEX format;

    format.nChannels = static_cast<WORD>(aNumChannels);
    format.nSamplesPerSec = aSampleRate;
    format.wFormatTag = static_cast<WORD>(((aFormat == BufferFormat::FLOAT_32 || aFormat == BufferFormat::FLOAT_64) ? WAVE_FORMAT_IEEE_FLOAT : WAVE_FORMAT_PCM));

    if (aFormat == BufferFormat::U_INT_8) {
        format.wBitsPerSample = 8;
    } else if (aFormat == BufferFormat::S_INT_16) {
        format.wBitsPerSample = 16;
    } else if (aFormat == BufferFormat::S_INT_24) {
        format.wBitsPerSample = 24;
    } else if (aFormat == BufferFormat::S_INT_32) {
        format.wBitsPerSample = 32;
    } else if (aFormat == BufferFormat::FLOAT_32) {
        format.wBitsPerSample = 32;
    } else {
        format.wBitsPerSample = 64;
    }

    format.nBlockAlign = format.nChannels * (format.wBitsPerSample / 8);
    format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
    format.cbSize = 0;

    if (FAILED(pClient->Initialize(AUDCLNT_SHAREMODE_EXCLUSIVE, AUDCLNT_STREAMFLAGS_EVENTCALLBACK, latency, latency, &format, &eventContext))) {
        return Exit::FAILURE;
    }

    return Exit::SUCCESS;
}

Exit Device::closeStream(Stream& aStream)
{
    return Exit::FAILURE;
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

    if (FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), reinterpret_cast<LPVOID*>(&spEnumerator)))) {
        CoUninitialize();

        return Exit::FAILURE;
    }

    return Exit::SUCCESS;
}

Exit getDeviceCount(unsigned int& arDeviceCount, DeviceType aType)
{
    EDataFlow flow = (aType == DeviceType::PLAYBACK) ? eRender : eCapture;
    CComPtr<IMMDeviceCollection> pCollection = nullptr;

    if (FAILED(spEnumerator->EnumAudioEndpoints(flow, DEVICE_STATE_ACTIVE, &pCollection))) {
        return Exit::FAILURE;
    }

    if (FAILED(pCollection->GetCount(&arDeviceCount))) {
        return Exit::FAILURE;
    }

    return Exit::SUCCESS;
}

Exit openDefaultDevice(Device& arDevice, DeviceType aType)
{
    EDataFlow flow = (aType == DeviceType::PLAYBACK) ? eRender : eCapture;
    CComPtr<IPropertyStore> pStore = nullptr;
    PROPVARIANT variant;
    WAVEFORMATEX* pQueryFormat = nullptr;
    std::array<DWORD, 15> standardSampleRates = { 8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000, 64000, 88200, 96000, 128000, 176400, 192000 };

    if (FAILED(spEnumerator->GetDefaultAudioEndpoint(flow, eMultimedia, &arDevice.pDevice))) {
        return Exit::FAILURE;
    }

    if (FAILED(arDevice.pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, reinterpret_cast<void**>(&arDevice.pClient)))) {
        return Exit::FAILURE;
    }

    if (FAILED(arDevice.pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, nullptr, reinterpret_cast<void**>(&arDevice.pVolume)))) {
        return Exit::FAILURE;
    }

    if (FAILED(CoCreateGuid(&arDevice.eventContext))) {
        return Exit::FAILURE;
    }

    if (FAILED(spEnumerator->RegisterEndpointNotificationCallback(&arDevice))) {
        return Exit::FAILURE;
    }

    if (FAILED(arDevice.pVolume->RegisterControlChangeNotify(&arDevice))) {
        spEnumerator->UnregisterEndpointNotificationCallback(&arDevice);

        return Exit::FAILURE;
    }

    if (FAILED(arDevice.pDevice->OpenPropertyStore(STGM_READ, &pStore))) {
        spEnumerator->UnregisterEndpointNotificationCallback(&arDevice);
        arDevice.pVolume->UnregisterControlChangeNotify(&arDevice);

        return Exit::FAILURE;
    }

    if (FAILED(pStore->GetValue(PKEY_Device_FriendlyName, &variant))) {
        spEnumerator->UnregisterEndpointNotificationCallback(&arDevice);
        arDevice.pVolume->UnregisterControlChangeNotify(&arDevice);

        return Exit::FAILURE;
    }

    arDevice.name = CW2A(variant.pwszVal);
    arDevice.type = aType;

    PropVariantClear(&variant);

    if (FAILED(pStore->GetValue(PKEY_AudioEngine_DeviceFormat, &variant))) {
        spEnumerator->UnregisterEndpointNotificationCallback(&arDevice);
        arDevice.pVolume->UnregisterControlChangeNotify(&arDevice);

        return Exit::FAILURE;
    }

    pQueryFormat = reinterpret_cast<WAVEFORMATEX*>(variant.blob.pBlobData);

    // Query supported channels
    for (WORD numChannels = 1; numChannels < 10; numChannels++) {
        pQueryFormat->nChannels = numChannels;

        if (SUCCEEDED(arDevice.pClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, pQueryFormat, nullptr))) {
            arDevice.supportedChannels.push_back(numChannels);
        }
    }

    if (arDevice.supportedChannels.size() == 0) {
        PropVariantClear(&variant);
        spEnumerator->UnregisterEndpointNotificationCallback(&arDevice);
        arDevice.pVolume->UnregisterControlChangeNotify(&arDevice);

        return Exit::FAILURE;
    }

    // Set nChannels to a valid value
    pQueryFormat->nChannels = static_cast<WORD>(arDevice.supportedChannels[0]);

    // Query supported sample rates
    for (DWORD sampleRate : standardSampleRates) {
        pQueryFormat->nSamplesPerSec = sampleRate;

        if (SUCCEEDED(arDevice.pClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, pQueryFormat, nullptr))) {
            arDevice.sampleRates.push_back(sampleRate);
        }
    }

    if (arDevice.sampleRates.size() == 0) {
        PropVariantClear(&variant);
        spEnumerator->UnregisterEndpointNotificationCallback(&arDevice);
        arDevice.pVolume->UnregisterControlChangeNotify(&arDevice);

        return Exit::FAILURE;
    }

    // Set nSamplesPerSec to a valid value
    pQueryFormat->nSamplesPerSec = arDevice.sampleRates[0];

    // Query supported buffer formats
    pQueryFormat->wFormatTag = WAVE_FORMAT_PCM;

    pQueryFormat->wBitsPerSample = 8;

    if (SUCCEEDED(arDevice.pClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, pQueryFormat, nullptr))) {
        arDevice.supportedFormats |= BufferFormat::U_INT_8;
    }

    pQueryFormat->wBitsPerSample = 16;

    if (SUCCEEDED(arDevice.pClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, pQueryFormat, nullptr))) {
        arDevice.supportedFormats |= BufferFormat::S_INT_16;
    }

    pQueryFormat->wBitsPerSample = 24;

    if (SUCCEEDED(arDevice.pClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, pQueryFormat, nullptr))) {
        arDevice.supportedFormats |= BufferFormat::S_INT_24;
    }

    pQueryFormat->wBitsPerSample = 32;

    if (SUCCEEDED(arDevice.pClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, pQueryFormat, nullptr))) {
        arDevice.supportedFormats |= BufferFormat::S_INT_32;
    }

    pQueryFormat->wFormatTag = WAVE_FORMAT_IEEE_FLOAT;

    if (SUCCEEDED(arDevice.pClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, pQueryFormat, nullptr))) {
        arDevice.supportedFormats |= BufferFormat::FLOAT_32;
    }

    pQueryFormat->wBitsPerSample = 64;

    if (SUCCEEDED(arDevice.pClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, pQueryFormat, nullptr))) {
        arDevice.supportedFormats |= BufferFormat::FLOAT_64;
    }

    if (arDevice.supportedFormats == static_cast<BufferFormat>(0)) {
        PropVariantClear(&variant);
        spEnumerator->UnregisterEndpointNotificationCallback(&arDevice);
        arDevice.pVolume->UnregisterControlChangeNotify(&arDevice);

        return Exit::FAILURE;
    }

    PropVariantClear(&variant);

    return Exit::SUCCESS;
}

Exit openDevice(Device& arDevice, DeviceType aType, unsigned int aIndex)
{
    EDataFlow flow = (aType == DeviceType::PLAYBACK) ? eRender : eCapture;
    CComPtr<IMMDeviceCollection> pCollection = nullptr;
    unsigned int count;
    CComPtr<IPropertyStore> pStore = nullptr;
    PROPVARIANT variant;
    WAVEFORMATEX* pQueryFormat = nullptr;
    std::array<DWORD, 15> standardSampleRates = { 8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000, 64000, 88200, 96000, 128000, 176400, 192000 };

    if (FAILED(spEnumerator->EnumAudioEndpoints(flow, DEVICE_STATE_ACTIVE, &pCollection))) {
        return Exit::FAILURE;
    }

    if (FAILED(pCollection->GetCount(&count))) {
        return Exit::FAILURE;
    }

    if (aIndex >= count) {
        return Exit::FAILURE;
    }

    if (FAILED(pCollection->Item(aIndex, &arDevice.pDevice))) {
        return Exit::FAILURE;
    }

    if (FAILED(arDevice.pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, reinterpret_cast<void**>(&arDevice.pClient)))) {
        return Exit::FAILURE;
    }

    if (FAILED(arDevice.pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, nullptr, reinterpret_cast<void**>(&arDevice.pVolume)))) {
        return Exit::FAILURE;
    }

    if (FAILED(CoCreateGuid(&arDevice.eventContext))) {
        return Exit::FAILURE;
    }

    if (FAILED(spEnumerator->RegisterEndpointNotificationCallback(&arDevice))) {
        return Exit::FAILURE;
    }

    if (FAILED(arDevice.pVolume->RegisterControlChangeNotify(&arDevice))) {
        spEnumerator->UnregisterEndpointNotificationCallback(&arDevice);

        return Exit::FAILURE;
    }

    if (FAILED(arDevice.pDevice->OpenPropertyStore(STGM_READ, &pStore))) {
        spEnumerator->UnregisterEndpointNotificationCallback(&arDevice);
        arDevice.pVolume->UnregisterControlChangeNotify(&arDevice);

        return Exit::FAILURE;
    }

    if (FAILED(pStore->GetValue(PKEY_Device_FriendlyName, &variant))) {
        spEnumerator->UnregisterEndpointNotificationCallback(&arDevice);
        arDevice.pVolume->UnregisterControlChangeNotify(&arDevice);

        return Exit::FAILURE;
    }

    arDevice.name = CW2A(variant.pwszVal);
    arDevice.type = aType;

    PropVariantClear(&variant);

    if (FAILED(pStore->GetValue(PKEY_AudioEngine_DeviceFormat, &variant))) {
        spEnumerator->UnregisterEndpointNotificationCallback(&arDevice);
        arDevice.pVolume->UnregisterControlChangeNotify(&arDevice);

        return Exit::FAILURE;
    }

    pQueryFormat = reinterpret_cast<WAVEFORMATEX*>(variant.blob.pBlobData);

    // Query supported channels
    for (WORD numChannels = 1; numChannels < 10; numChannels++) {
        pQueryFormat->nChannels = numChannels;

        if (SUCCEEDED(arDevice.pClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, pQueryFormat, nullptr))) {
            arDevice.supportedChannels.push_back(numChannels);
        }
    }

    if (arDevice.supportedChannels.size() == 0) {
        PropVariantClear(&variant);
        spEnumerator->UnregisterEndpointNotificationCallback(&arDevice);
        arDevice.pVolume->UnregisterControlChangeNotify(&arDevice);

        return Exit::FAILURE;
    }

    // Set nChannels to a valid value
    pQueryFormat->nChannels = static_cast<WORD>(arDevice.supportedChannels[0]);

    // Query supported sample rates
    for (DWORD sampleRate : standardSampleRates) {
        pQueryFormat->nSamplesPerSec = sampleRate;

        if (SUCCEEDED(arDevice.pClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, pQueryFormat, nullptr))) {
            arDevice.sampleRates.push_back(sampleRate);
        }
    }

    if (arDevice.sampleRates.size() == 0) {
        PropVariantClear(&variant);
        spEnumerator->UnregisterEndpointNotificationCallback(&arDevice);
        arDevice.pVolume->UnregisterControlChangeNotify(&arDevice);

        return Exit::FAILURE;
    }

    // Set nSamplesPerSec to a valid value
    pQueryFormat->nSamplesPerSec = arDevice.sampleRates[0];

    // Query supported buffer formats
    pQueryFormat->wFormatTag = WAVE_FORMAT_PCM;

    pQueryFormat->wBitsPerSample = 8;

    if (SUCCEEDED(arDevice.pClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, pQueryFormat, nullptr))) {
        arDevice.supportedFormats |= BufferFormat::U_INT_8;
    }

    pQueryFormat->wBitsPerSample = 16;

    if (SUCCEEDED(arDevice.pClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, pQueryFormat, nullptr))) {
        arDevice.supportedFormats |= BufferFormat::S_INT_16;
    }

    pQueryFormat->wBitsPerSample = 24;

    if (SUCCEEDED(arDevice.pClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, pQueryFormat, nullptr))) {
        arDevice.supportedFormats |= BufferFormat::S_INT_24;
    }

    pQueryFormat->wBitsPerSample = 32;

    if (SUCCEEDED(arDevice.pClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, pQueryFormat, nullptr))) {
        arDevice.supportedFormats |= BufferFormat::S_INT_32;
    }

    pQueryFormat->wFormatTag = WAVE_FORMAT_IEEE_FLOAT;

    if (SUCCEEDED(arDevice.pClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, pQueryFormat, nullptr))) {
        arDevice.supportedFormats |= BufferFormat::FLOAT_32;
    }

    pQueryFormat->wBitsPerSample = 64;

    if (SUCCEEDED(arDevice.pClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, pQueryFormat, nullptr))) {
        arDevice.supportedFormats |= BufferFormat::FLOAT_64;
    }

    if (arDevice.supportedFormats == static_cast<BufferFormat>(0)) {
        PropVariantClear(&variant);
        spEnumerator->UnregisterEndpointNotificationCallback(&arDevice);
        arDevice.pVolume->UnregisterControlChangeNotify(&arDevice);

        return Exit::FAILURE;
    }

    PropVariantClear(&variant);

    return Exit::SUCCESS;
}

Exit openDevice(Device& arDevice, std::string aDeviceName)
{
    CComPtr<IMMDeviceCollection> pCollection = nullptr;
    unsigned int count;
    PROPVARIANT variant;
    std::string deviceName;
    EDataFlow flow;
    WAVEFORMATEX* pQueryFormat = nullptr;
    std::array<DWORD, 15> standardSampleRates = { 8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000, 64000, 88200, 96000, 128000, 176400, 192000 };

    if (FAILED(spEnumerator->EnumAudioEndpoints(eAll, DEVICE_STATE_ACTIVE, &pCollection))) {
        return Exit::FAILURE;
    }

    if (FAILED(pCollection->GetCount(&count))) {
        return Exit::FAILURE;
    }

    // Scan the list of available devices
    for (unsigned int i = 0; i < count; i++) {
        CComPtr<IPropertyStore> pStore = nullptr;
        CComPtr<IMMEndpoint> pEndpoint = nullptr;

        if (FAILED(pCollection->Item(i, &arDevice.pDevice))) {
            return Exit::FAILURE;
        }

        if (FAILED(arDevice.pDevice->OpenPropertyStore(STGM_READ, &pStore))) {
            return Exit::FAILURE;
        }

        if (FAILED(pStore->GetValue(PKEY_Device_FriendlyName, &variant))) {
            return Exit::FAILURE;
        }

        deviceName = CW2A(variant.pwszVal);

        // If device is found
        if (aDeviceName == deviceName) {
            if (FAILED(arDevice.pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, reinterpret_cast<void**>(&arDevice.pClient)))) {
                PropVariantClear(&variant);

                return Exit::FAILURE;
            }

            if (FAILED(arDevice.pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, nullptr, reinterpret_cast<void**>(&arDevice.pVolume)))) {
                PropVariantClear(&variant);

                return Exit::FAILURE;
            }

            if (FAILED(CoCreateGuid(&arDevice.eventContext))) {
                PropVariantClear(&variant);

                return Exit::FAILURE;
            }

            if (FAILED(spEnumerator->RegisterEndpointNotificationCallback(&arDevice))) {
                PropVariantClear(&variant);

                return Exit::FAILURE;
            }

            if (FAILED(arDevice.pVolume->RegisterControlChangeNotify(&arDevice))) {
                spEnumerator->UnregisterEndpointNotificationCallback(&arDevice);
                PropVariantClear(&variant);

                return Exit::FAILURE;
            }

            if (FAILED(arDevice.pDevice->QueryInterface(__uuidof(IMMEndpoint), reinterpret_cast<void**>(&pEndpoint)))) {
                spEnumerator->UnregisterEndpointNotificationCallback(&arDevice);
                arDevice.pVolume->UnregisterControlChangeNotify(&arDevice);
                PropVariantClear(&variant);

                return Exit::FAILURE;
            }

            if (FAILED(pEndpoint->GetDataFlow(&flow))) {
                spEnumerator->UnregisterEndpointNotificationCallback(&arDevice);
                arDevice.pVolume->UnregisterControlChangeNotify(&arDevice);
                PropVariantClear(&variant);

                return Exit::FAILURE;
            }

            arDevice.name = deviceName;
            arDevice.type = (flow == eRender) ? DeviceType::PLAYBACK : DeviceType::CAPTURE;

            PropVariantClear(&variant);

            if (FAILED(pStore->GetValue(PKEY_AudioEngine_DeviceFormat, &variant))) {
                spEnumerator->UnregisterEndpointNotificationCallback(&arDevice);
                arDevice.pVolume->UnregisterControlChangeNotify(&arDevice);

                return Exit::FAILURE;
            }

            pQueryFormat = reinterpret_cast<WAVEFORMATEX*>(variant.blob.pBlobData);

            // Query supported channels
            for (WORD numChannels = 1; numChannels < 10; numChannels++) {
                pQueryFormat->nChannels = numChannels;

                if (SUCCEEDED(arDevice.pClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, pQueryFormat, nullptr))) {
                    arDevice.supportedChannels.push_back(numChannels);
                }
            }

            if (arDevice.supportedChannels.size() == 0) {
                PropVariantClear(&variant);
                spEnumerator->UnregisterEndpointNotificationCallback(&arDevice);
                arDevice.pVolume->UnregisterControlChangeNotify(&arDevice);

                return Exit::FAILURE;
            }

            // Set nChannels to a valid value
            pQueryFormat->nChannels = static_cast<WORD>(arDevice.supportedChannels[0]);

            // Query supported sample rates
            for (DWORD sampleRate : standardSampleRates) {
                pQueryFormat->nSamplesPerSec = sampleRate;

                if (SUCCEEDED(arDevice.pClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, pQueryFormat, nullptr))) {
                    arDevice.sampleRates.push_back(sampleRate);
                }
            }

            if (arDevice.sampleRates.size() == 0) {
                PropVariantClear(&variant);
                spEnumerator->UnregisterEndpointNotificationCallback(&arDevice);
                arDevice.pVolume->UnregisterControlChangeNotify(&arDevice);

                return Exit::FAILURE;
            }

            // Set nSamplesPerSec to a valid value
            pQueryFormat->nSamplesPerSec = arDevice.sampleRates[0];

            // Query supported buffer formats
            pQueryFormat->wFormatTag = WAVE_FORMAT_PCM;

            pQueryFormat->wBitsPerSample = 8;

            if (SUCCEEDED(arDevice.pClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, pQueryFormat, nullptr))) {
                arDevice.supportedFormats |= BufferFormat::U_INT_8;
            }

            pQueryFormat->wBitsPerSample = 16;

            if (SUCCEEDED(arDevice.pClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, pQueryFormat, nullptr))) {
                arDevice.supportedFormats |= BufferFormat::S_INT_16;
            }

            pQueryFormat->wBitsPerSample = 24;

            if (SUCCEEDED(arDevice.pClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, pQueryFormat, nullptr))) {
                arDevice.supportedFormats |= BufferFormat::S_INT_24;
            }

            pQueryFormat->wBitsPerSample = 32;

            if (SUCCEEDED(arDevice.pClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, pQueryFormat, nullptr))) {
                arDevice.supportedFormats |= BufferFormat::S_INT_32;
            }

            pQueryFormat->wFormatTag = WAVE_FORMAT_IEEE_FLOAT;

            if (SUCCEEDED(arDevice.pClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, pQueryFormat, nullptr))) {
                arDevice.supportedFormats |= BufferFormat::FLOAT_32;
            }

            pQueryFormat->wBitsPerSample = 64;

            if (SUCCEEDED(arDevice.pClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, pQueryFormat, nullptr))) {
                arDevice.supportedFormats |= BufferFormat::FLOAT_64;
            }

            if (arDevice.supportedFormats == static_cast<BufferFormat>(0)) {
                PropVariantClear(&variant);
                spEnumerator->UnregisterEndpointNotificationCallback(&arDevice);
                arDevice.pVolume->UnregisterControlChangeNotify(&arDevice);

                return Exit::FAILURE;
            }

            PropVariantClear(&variant);

            return Exit::SUCCESS;
        }

        // Cleanup for reuse
        PropVariantClear(&variant);
    }

    PropVariantClear(&variant);

    return Exit::FAILURE;
}

Exit closeDevice(Device& arDevice)
{
    if (arDevice.pDevice == nullptr || arDevice.pVolume == nullptr) {
        return Exit::FAILURE;
    }

    if (FAILED(arDevice.pVolume->UnregisterControlChangeNotify(&arDevice))) {
        return Exit::FAILURE;
    }

    if (FAILED(spEnumerator->UnregisterEndpointNotificationCallback(&arDevice))) {
        return Exit::FAILURE;
    }

    return Exit::SUCCESS;
}

Exit quit()
{
    // If BriskAudio is already uninitialized/not initialized
    if (spEnumerator == nullptr) {
        return Exit::FAILURE;
    }

    // CoUninitialize() must be called only once when the program exits
    atexit(reinterpret_cast<void(__cdecl*)()>(CoUninitialize));

    return Exit::SUCCESS;
}
}
#endif
