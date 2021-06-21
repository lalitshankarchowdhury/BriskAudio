#ifdef _WIN32
#include "BriskAudio.hpp"
#include <atlstr.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>

#define EXIT_ON_ERROR(hres)  \
              if (FAILED(hres)) { hres = S_OK; goto Exit; }
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

namespace BriskAudio {
    Exit init() {
        HRESULT result = S_OK;

        result = CoInitialize(NULL);
        EXIT_ON_ERROR(result)
    
    Exit:
        return Exit::SUCCESS;
    }

    unsigned int DeviceInfoCollection::getDeviceCount(DeviceType aType) {
        HRESULT result = S_OK;
        IMMDeviceEnumerator* enumerator = NULL;
        IMMDeviceCollection* collection = NULL;
        EDataFlow flow;
        unsigned int count = 0;

        result = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**) &enumerator);
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
        IMMDevice* pDevice = NULL;
        IMMDeviceEnumerator* enumerator = NULL;
        IMMDeviceCollection* collection = NULL;
        
        EDataFlow flow;
        unsigned int count;
        DeviceInfo temp;
        IPropertyStore *pStore = NULL;
        PROPVARIANT varName;
        LPWSTR pwszID = NULL;

        result = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**) &enumerator);
        EXIT_ON_ERROR(result)

        flow = (aType == DeviceType::PLAYBACK)? eRender : eCapture;

        result = enumerator->EnumAudioEndpoints(flow, DEVICE_STATEMASK_ALL, &collection);
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

        temp.name = CW2A(varName.pwszVal);

        result = pDevice->GetId(&pwszID);
        EXIT_ON_ERROR(result)

        temp.ID = CW2A(pwszID);

        result = PropVariantClear(&varName);

        result = pStore->GetValue(PKEY_Device_DeviceDesc, &varName);
        EXIT_ON_ERROR(result)

        temp.description = CW2A(varName.pwszVal);

        temp.isValid = true;

    Exit:
        CoTaskMemFree(pwszID);
        SAFE_RELEASE(enumerator)
        SAFE_RELEASE(collection)
        SAFE_RELEASE(pDevice)
        SAFE_RELEASE(pStore)

        return temp;
    }

    void quit() {
        CoUninitialize();
    }
}
#endif
