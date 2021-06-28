#ifdef _WIN32
#include "../include/BriskAudio.hpp"
#include <atlstr.h>
#include <Audioclient.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>

static HANDLE shConsoleHandle = nullptr;
static DWORD sDefaultConsoleMode;
static IMMDeviceEnumerator* spEnumerator = nullptr;

namespace BriskAudio {
Exit init()
{
    if ((shConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE)) == nullptr) {
        return Exit::FAILURE;
    }

    if (GetConsoleMode(shConsoleHandle, &sDefaultConsoleMode) == FALSE) {
        return Exit::FAILURE;
    }

    // Enable colored output
    if (SetConsoleMode(shConsoleHandle, sDefaultConsoleMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING) == FALSE) {
        return Exit::FAILURE;
    }

    if (FAILED(CoInitialize(nullptr))) {
        return Exit::FAILURE;
    }

    if (FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&spEnumerator))) {
        CoUninitialize();

        return Exit::FAILURE;
    }

    return Exit::SUCCESS;
}

void Endpoint::releaseNativeHandle()
{
    if (nativeHandle != nullptr) {
        ((IMMDevice*)nativeHandle)->Release();

        nativeHandle = nullptr;
    }
}

StreamConfig Endpoint::getSupportedStreamConfigs()
{
    IAudioClient* pClient = nullptr;
    WAVEFORMATEX* pFormat = nullptr;
    StreamConfig config;

    if (FAILED(((IMMDevice*)nativeHandle)->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void**)&pClient))) {
        return StreamConfig();
    }

    if (FAILED(pClient->GetMixFormat(&pFormat))) {
        pClient->Release();

        return StreamConfig();
    }

    config.numChannels = pFormat->nChannels;
    config.sampleRate = pFormat->nSamplesPerSec;

    if (pFormat->wFormatTag == WAVE_FORMAT_PCM) {
        if (pFormat->wBitsPerSample == 8) {
            config.format = BufferFormat::U_INT_8;
        }
        else if (pFormat->wBitsPerSample == 16) {
            config.format = BufferFormat::S_INT_16;
        }
        else if (pFormat->wBitsPerSample == 24) {
            config.format = BufferFormat::S_INT_24;
        }
        else {
            config.format = BufferFormat::S_INT_32;
        }
    }
    else {
        if (pFormat->wBitsPerSample == 32) {
            config.format = BufferFormat::FLOAT_32;
        }
        else {
            config.format = BufferFormat::FLOAT_64;
        }
    }

    config.isValid = true;

    pClient->Release();
    free(pFormat);

    return config;
}

Exit Endpoint::openStream(Stream* aStream)
{
    if (aStream == nullptr) {
        return Exit::FAILURE;
    }

    if (FAILED(((IMMDevice*)nativeHandle)->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void**)&aStream->nativeHandle))) {
        return Exit::FAILURE;
    }

    return Exit::SUCCESS;
}

Exit Endpoint::closeStream(Stream* aStream)
{
    if (aStream == nullptr) {
        return Exit::FAILURE;
    }

    ((IAudioClient*)aStream->nativeHandle)->Release();

    return Exit::SUCCESS;
}

unsigned int EndpointEnumerator::getEndpointCount()
{
    EDataFlow flow = (type == EndpointType::PLAYBACK) ? eRender : eCapture;
    IMMDeviceCollection* pCollection = nullptr;
    unsigned int count;

    if (FAILED(spEnumerator->EnumAudioEndpoints(flow, DEVICE_STATE_ACTIVE, &pCollection))) {
        return 0;
    }

    if (FAILED(pCollection->GetCount(&count))) {
        pCollection->Release();

        return 0;
    }

    pCollection->Release();

    return count;
}

Endpoint EndpointEnumerator::getDefaultEndpoint()
{
    EDataFlow flow = (type == EndpointType::PLAYBACK) ? eRender : eCapture;
    IPropertyStore* pStore = nullptr;
    PROPVARIANT varName;
    Endpoint endpoint;

    if (FAILED(spEnumerator->GetDefaultAudioEndpoint(flow, eConsole, (IMMDevice**)&endpoint.nativeHandle))) {
        return Endpoint();
    }

    if (FAILED(((IMMDevice*)endpoint.nativeHandle)->OpenPropertyStore(STGM_READ, &pStore))) {
        endpoint.releaseNativeHandle();

        return Endpoint();
    }

    if (FAILED(pStore->GetValue(PKEY_DeviceInterface_FriendlyName, &varName))) {
        pStore->Release();

        endpoint.releaseNativeHandle();

        return Endpoint();
    }

    endpoint.cardName = CW2A(varName.pwszVal);

    if (FAILED(PropVariantClear(&varName))) {
        pStore->Release();

        endpoint.releaseNativeHandle();

        return Endpoint();
    }

    if (FAILED(pStore->GetValue(PKEY_Device_DeviceDesc, &varName))) {
        pStore->Release();

        endpoint.releaseNativeHandle();

        return Endpoint();
    }

    endpoint.description = CW2A(varName.pwszVal);
    endpoint.isValid = true;

    pStore->Release();

    return endpoint;
}

Endpoint EndpointEnumerator::getEndpoint(unsigned int aIndex)
{
    EDataFlow flow = (type == EndpointType::PLAYBACK) ? eRender : eCapture;
    IMMDeviceCollection* pCollection = nullptr;
    unsigned int count;
    IPropertyStore* pStore = nullptr;
    PROPVARIANT varName;
    Endpoint endpoint;

    if (FAILED(spEnumerator->EnumAudioEndpoints(flow, DEVICE_STATE_ACTIVE, &pCollection))) {
        return Endpoint();
    }

    if (FAILED(pCollection->GetCount(&count))) {
        pCollection->Release();

        return Endpoint();
    }

    if (aIndex >= count) {
        pCollection->Release();

        return Endpoint();
    }

    if (FAILED(pCollection->Item(aIndex, (IMMDevice**)&endpoint.nativeHandle))) {
        pCollection->Release();

        return Endpoint();
    }

    if (FAILED(((IMMDevice*)endpoint.nativeHandle)->OpenPropertyStore(STGM_READ, &pStore))) {
        endpoint.releaseNativeHandle();

        pCollection->Release();

        return Endpoint();
    }

    if (FAILED(pStore->GetValue(PKEY_DeviceInterface_FriendlyName, &varName))) {
        pStore->Release();

        endpoint.releaseNativeHandle();

        pCollection->Release();

        return Endpoint();
    }

    endpoint.cardName = CW2A(varName.pwszVal);

    if (FAILED(PropVariantClear(&varName))) {
        pStore->Release();

        endpoint.releaseNativeHandle();

        pCollection->Release();

        return Endpoint();
    }

    if (FAILED(pStore->GetValue(PKEY_Device_DeviceDesc, &varName))) {
        pStore->Release();

        endpoint.releaseNativeHandle();

        pCollection->Release();

        return Endpoint();
    }

    endpoint.description = CW2A(varName.pwszVal);
    endpoint.type = type;
    endpoint.isValid = true;

    pStore->Release();
    pCollection->Release();

    return endpoint;
}

Exit quit()
{
    spEnumerator->Release();

    spEnumerator = nullptr;

    CoUninitialize();

    if (SetConsoleMode(shConsoleHandle, sDefaultConsoleMode) == TRUE) {
        shConsoleHandle = nullptr;

        return Exit::SUCCESS;
    }
    else {
        shConsoleHandle = nullptr;

        return Exit::SUCCESS;
    }
}
}
#endif
