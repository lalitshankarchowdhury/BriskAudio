#ifdef _WIN32
#include "BriskAudio.hpp"
#include <atlstr.h>
#include <Audioclient.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>

#define EXIT_ON_ERROR(hres)  \
              if (FAILED(hres)) { hres = S_OK; goto Exit; }
#define SAFE_RELEASE(punk)  \
              if ((punk) != nullptr)  \
                { (punk)->Release(); (punk) = nullptr; }

namespace BriskAudio {
    Exit init() {
        HANDLE hConsoleHandle = NULL;
        DWORD consoleMode;

        hConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

        if (hConsoleHandle == NULL) {
            return Exit::FAILURE;
        }

        if (!GetConsoleMode(hConsoleHandle, &consoleMode)) {
            return Exit::FAILURE;
        }

        // Allow colored output
        consoleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

        if (!SetConsoleMode(hConsoleHandle, consoleMode)) {
            return Exit::FAILURE;
        }

       if (FAILED(CoInitialize(NULL))) {
           return Exit::FAILURE;
       }

       return Exit::SUCCESS;
    }

    unsigned int DeviceInfoCollection::getDeviceCount(DeviceType aType) {
        HRESULT result = S_OK;
        IMMDeviceEnumerator* enumerator = nullptr;
        IMMDeviceCollection* collection = nullptr;
        EDataFlow flow;
        unsigned int count = 0;

        result = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**) &enumerator);
        EXIT_ON_ERROR(result)

        flow = (aType == DeviceType::PLAYBACK)? eRender : eCapture;

        result = enumerator->EnumAudioEndpoints(flow, DEVICE_STATE_ACTIVE, &collection);
        EXIT_ON_ERROR(result)

        result = collection->GetCount(&count);
        EXIT_ON_ERROR(result)

    Exit:
        SAFE_RELEASE(enumerator)
        SAFE_RELEASE(collection)

        return count;
    }

    DeviceInfo DeviceInfoCollection::getDeviceInfo(unsigned int aIndex, DeviceType aType) {
        HRESULT result = S_OK;
        IMMDevice* pDevice = nullptr;
        IMMDeviceEnumerator* enumerator = nullptr;
        IMMDeviceCollection* collection = nullptr;
        IAudioClient* pClient = nullptr;
        WAVEFORMATEX* pFormat;
        
        EDataFlow flow;
        unsigned int count;
        DeviceInfo info;
        IPropertyStore *pStore = nullptr;
        PROPVARIANT varName;

        result = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**) &enumerator);
        EXIT_ON_ERROR(result)

        flow = (aType == DeviceType::PLAYBACK)? eRender : eCapture;

        result = enumerator->EnumAudioEndpoints(flow, DEVICE_STATE_ACTIVE, &collection);
        EXIT_ON_ERROR(result)

        result = collection->GetCount(&count);
        EXIT_ON_ERROR(result)

        if (aIndex < 0 || aIndex >= count) {
            goto Exit;
        }

        result = collection->Item(aIndex, &pDevice);
        EXIT_ON_ERROR(result)

        result = pDevice->OpenPropertyStore(STGM_READ, &pStore);
        EXIT_ON_ERROR(result)

        PropVariantInit(&varName);

        result = pStore->GetValue(PKEY_DeviceInterface_FriendlyName, &varName);
        EXIT_ON_ERROR(result)

        info.name = CW2A(varName.pwszVal);

        result = PropVariantClear(&varName);

        result = pStore->GetValue(PKEY_Device_DeviceDesc, &varName);
        EXIT_ON_ERROR(result)

        info.description = CW2A(varName.pwszVal);

        info.type = aType;

        result = pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**) &pClient);
        EXIT_ON_ERROR(result)

        result = pClient->GetMixFormat(&pFormat);
        EXIT_ON_ERROR(result)

        info.numChannels = pFormat->nChannels;

        info.defaultSampleRate = pFormat->nSamplesPerSec;

        info.isValid = true;

    Exit:
        SAFE_RELEASE(enumerator)
        SAFE_RELEASE(collection)
        SAFE_RELEASE(pDevice)
        SAFE_RELEASE(pStore)
        SAFE_RELEASE(pClient)

        return info;
    }

    void quit() {
        CoUninitialize();
    }
}
#endif
