#ifdef _WIN32
#include "BriskAudio.hpp"
#include <assert.h>
#include <atlstr.h>
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
    endpoint.type = type;
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

    // Reset console mode
    return (SetConsoleMode(shConsoleHandle, sDefaultConsoleMode) == TRUE) ? Exit::SUCCESS : Exit::FAILURE;
}
}
#endif
