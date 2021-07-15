#ifdef _WIN32
#include "../include/BriskAudio.hpp"
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
    eventContext = GUID_NULL;
    cRef_ = 1;
    pOnVolumeChange = nullptr;
    pOnMute = nullptr;
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

        *ppvInterface = (IUnknown*)this;
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
    WAVEFORMATEX format;
    IAudioClient* pClient = nullptr;

    if (FAILED(pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void**)&pClient))) {
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

    if (FAILED(spEnumerator->GetDefaultAudioEndpoint(flow, eMultimedia, &arDevice.pDevice))) {
        return Exit::FAILURE;
    }

    if (FAILED(arDevice.pDevice->OpenPropertyStore(STGM_READ, &pStore))) {
        arDevice.pDevice->Release();

        return Exit::FAILURE;
    }

    if (FAILED(pStore->GetValue(PKEY_Device_FriendlyName, &varName))) {
        pStore->Release();
        arDevice.pDevice->Release();

        return Exit::FAILURE;
    }

    arDevice.name = CW2A(varName.pwszVal);

    if (FAILED(arDevice.pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, nullptr, (void**)&arDevice.pVolume))) {
        PropVariantClear(&varName);
        pStore->Release();
        arDevice.pDevice->Release();
        return Exit::FAILURE;
    }

    if (FAILED(arDevice.pVolume->RegisterControlChangeNotify(&arDevice))) {
        arDevice.pVolume->Release();
        PropVariantClear(&varName);
        pStore->Release();
        arDevice.pDevice->Release();

        return Exit::FAILURE;
    }

    if (FAILED(CoCreateGuid(&arDevice.eventContext))) {
        arDevice.pVolume->Release();
        PropVariantClear(&varName);
        pStore->Release();
        arDevice.pDevice->Release();

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

    if (FAILED(pCollection->Item(aIndex, &arDevice.pDevice))) {
        pCollection->Release();

        return Exit::FAILURE;
    }

    if (FAILED(arDevice.pDevice->OpenPropertyStore(STGM_READ, &pStore))) {
        arDevice.pDevice->Release();
        pCollection->Release();

        return Exit::FAILURE;
    }

    if (FAILED(pStore->GetValue(PKEY_Device_FriendlyName, &varName))) {
        pStore->Release();
        arDevice.pDevice->Release();
        pCollection->Release();

        return Exit::FAILURE;
    }

    arDevice.name = CW2A(varName.pwszVal);

    if (FAILED(arDevice.pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, nullptr, (void**)&arDevice.pVolume))) {
        PropVariantClear(&varName);
        pStore->Release();
        arDevice.pDevice->Release();
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

            if (FAILED(arDevice.pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, nullptr, (void**)&arDevice.pVolume))) {
                pEndpoint->Release();
                PropVariantClear(&varName);
                pStore->Release();
                pDevice->Release();
                pCollection->Release();

                return Exit::FAILURE;
            }

            arDevice.name = deviceName;
            arDevice.type = (flow == eRender) ? DeviceType::PLAYBACK : DeviceType::CAPTURE;
            arDevice.pDevice = pDevice;

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
    if (arDevice.pDevice == nullptr || arDevice.pVolume == nullptr) {
        return Exit::FAILURE;
    }

    if (FAILED(arDevice.pVolume->RegisterControlChangeNotify(&arDevice))) {
        arDevice.pVolume->Release();
        arDevice.pVolume = nullptr;
        arDevice.pDevice->Release();
        arDevice.pDevice = nullptr;

        return Exit::FAILURE;
    }

    arDevice.pVolume->Release();
    arDevice.pVolume = nullptr;
    arDevice.pDevice->Release();
    arDevice.pDevice = nullptr;

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
