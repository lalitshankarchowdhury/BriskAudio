#ifdef _WIN32
#include "../include/BriskAudio.hpp"
#include <atlstr.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>

#define EXIT_ON_ERROR(hres)  \
              if (FAILED(hres)) { hres = S_OK; goto Exit; }
#define SAFE_RELEASE(punk)  \
              if (punk != nullptr)  \
                { (punk)->Release(); (punk) = nullptr; }

static HANDLE shConsoleHandle = nullptr;
static DWORD sDefaultConsoleMode;
static IMMDeviceEnumerator* spEnumerator = nullptr;

namespace BriskAudio {
    Exit init() {
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

        if (FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**) &spEnumerator))) {
            CoUninitialize();

            return Exit::FAILURE;
        }

        return Exit::SUCCESS;
    }

    unsigned int EndpointInfoCollection::getEndpointCount() {
        HRESULT result;
        EDataFlow flow;
        IMMDeviceCollection* pCollection = nullptr;
        unsigned int count = 0;

        flow = (type_ == EndpointType::PLAYBACK)? eRender : eCapture;

        result = spEnumerator->EnumAudioEndpoints(flow, DEVICE_STATE_ACTIVE, &pCollection);
        EXIT_ON_ERROR(result)

        result = pCollection->GetCount(&count);
        EXIT_ON_ERROR(result)

    Exit:
        SAFE_RELEASE(pCollection)

        return count;
    }

    EndpointInfo EndpointInfoCollection::getEndpointInfo(unsigned int aIndex) {
        HRESULT result;
        EDataFlow flow;
        IMMDeviceCollection* pCollection = nullptr;  
        IMMDevice* pDevice = nullptr;
        IPropertyStore *pStore = nullptr;
        PROPVARIANT varName;
        unsigned int count;
        EndpointInfo info;

        flow = (type_ == EndpointType::PLAYBACK)? eRender : eCapture;

        result = spEnumerator->EnumAudioEndpoints(flow, DEVICE_STATE_ACTIVE, &pCollection);
        EXIT_ON_ERROR(result)

        result = pCollection->GetCount(&count);
        EXIT_ON_ERROR(result)

        if (aIndex < 0 || aIndex >= count) {
            goto Exit;
        }

        result = pCollection->Item(aIndex, &pDevice);
        EXIT_ON_ERROR(result)

        result = pDevice->OpenPropertyStore(STGM_READ, &pStore);
        EXIT_ON_ERROR(result)

        PropVariantInit(&varName);

        result = pStore->GetValue(PKEY_DeviceInterface_FriendlyName, &varName);
        EXIT_ON_ERROR(result)

        info.cardName = CW2A(varName.pwszVal);

        result = PropVariantClear(&varName);

        result = pStore->GetValue(PKEY_Device_DeviceDesc, &varName);
        EXIT_ON_ERROR(result)

        info.description = CW2A(varName.pwszVal);

        info.type = type_;

        info.isValid = true;

    Exit:
        SAFE_RELEASE(pStore)
        SAFE_RELEASE(pDevice)
        SAFE_RELEASE(pCollection)        

        return info;
    }

    void quit() {
        SAFE_RELEASE(spEnumerator)

        CoUninitialize();

        // Reset console mode
        SetConsoleMode(shConsoleHandle, sDefaultConsoleMode);
    }
}
#endif
