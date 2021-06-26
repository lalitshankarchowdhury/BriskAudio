#ifdef _WIN32
#include "../include/BriskAudio.hpp"
#include <atlstr.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>

#define SAFE_RELEASE(punk) \
    if (punk != nullptr) { \
        (punk)->Release(); \
        (punk) = nullptr;  \
    }

static HANDLE shConsoleHandle = nullptr;
static DWORD sDefaultConsoleMode;
static IMMDeviceEnumerator* spEnumerator = nullptr;

namespace BriskAudio {
Exit init()
{
    shConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

    if (shConsoleHandle == nullptr) {
        return Exit::FAILURE;
    }

    if (!GetConsoleMode(shConsoleHandle, &sDefaultConsoleMode)) {
        return Exit::FAILURE;
    }

    // Enable colored output
    if (!SetConsoleMode(shConsoleHandle, sDefaultConsoleMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
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

unsigned int EndpointEnumerator::getEndpointCount()
{
    EDataFlow flow = (type == EndpointType::PLAYBACK) ? eRender : eCapture;
    IMMDeviceCollection* pCollection = nullptr;
    unsigned int count = 0;

    if (FAILED(spEnumerator->EnumAudioEndpoints(flow, DEVICE_STATE_ACTIVE, &pCollection))) {
        goto Exit;
    }

    if (FAILED(pCollection->GetCount(&count))) {
        goto Exit;
    }

Exit:
    SAFE_RELEASE(pCollection)

    return count;
}

Endpoint EndpointEnumerator::getDefaultEndpoint()
{
    EDataFlow flow = (type == EndpointType::PLAYBACK) ? eRender : eCapture;
    IPropertyStore* pStore = nullptr;
    PROPVARIANT varName;
    Endpoint endpoint;

    if (FAILED(spEnumerator->GetDefaultAudioEndpoint(flow, eConsole, (IMMDevice**)&endpoint.nativeHandle))) {
        goto Exit;
    }

    if (FAILED(((IMMDevice*)endpoint.nativeHandle)->OpenPropertyStore(STGM_READ, &pStore))) {
        endpoint.releaseNativeHandle();

        goto Exit;
    }

    if (FAILED(pStore->GetValue(PKEY_DeviceInterface_FriendlyName, &varName))) {
        endpoint.releaseNativeHandle();

        goto Exit;
    }

    endpoint.cardName = CW2A(varName.pwszVal);

    if (FAILED(PropVariantClear(&varName))) {
        endpoint.releaseNativeHandle();

        goto Exit;
    }

    if (FAILED(pStore->GetValue(PKEY_Device_DeviceDesc, &varName))) {
        endpoint.releaseNativeHandle();

        goto Exit;
    }

    endpoint.description = CW2A(varName.pwszVal);

    endpoint.type = type;

    endpoint.isValid = true;

Exit:
    SAFE_RELEASE(pStore)

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
        goto Exit;
    }

    if (FAILED(pCollection->GetCount(&count))) {
        goto Exit;
    }

    if (aIndex >= count) {
        goto Exit;
    }

    if (FAILED(pCollection->Item(aIndex, (IMMDevice**)&endpoint.nativeHandle))) {
        goto Exit;
    }

    if (FAILED(((IMMDevice*)endpoint.nativeHandle)->OpenPropertyStore(STGM_READ, &pStore))) {
        endpoint.releaseNativeHandle();

        goto Exit;
    }

    if (FAILED(pStore->GetValue(PKEY_DeviceInterface_FriendlyName, &varName))) {
        endpoint.releaseNativeHandle();

        goto Exit;
    }

    endpoint.cardName = CW2A(varName.pwszVal);

    if (FAILED(PropVariantClear(&varName))) {
        endpoint.releaseNativeHandle();

        goto Exit;
    }

    if (FAILED(pStore->GetValue(PKEY_Device_DeviceDesc, &varName))) {
        endpoint.releaseNativeHandle();

        goto Exit;
    }

    endpoint.description = CW2A(varName.pwszVal);

    endpoint.type = type;

    endpoint.isValid = true;

Exit:
    SAFE_RELEASE(pStore)
    SAFE_RELEASE(pCollection)

    return endpoint;
}

Exit quit()
{
    SAFE_RELEASE(spEnumerator)

    CoUninitialize();

    // Reset console mode
    return (SetConsoleMode(shConsoleHandle, sDefaultConsoleMode) == TRUE) ? Exit::SUCCESS : Exit::FAILURE;
}
}
#endif
