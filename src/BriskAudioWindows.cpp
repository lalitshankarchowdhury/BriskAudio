#ifdef _WIN32
#include "../include/BriskAudio.hpp"
#include <atlstr.h>
#include <Audioclient.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>

static IMMDeviceEnumerator* spEnumerator = nullptr;
static LPCWSTR spDeviceId = nullptr;

using namespace BriskAudio;

class CMMNotificationClient : public IMMNotificationClient {
    LONG _cRef;

public:
    CMMNotificationClient()
    {
        _cRef = 1;
    }

    ~CMMNotificationClient()
    {
        pOnDefaultDeviceChange = nullptr;
        pOnDeviceAdd = nullptr;
        pOnDeviceRemove = nullptr;
    }

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return InterlockedIncrement(&_cRef);
    }

    ULONG STDMETHODCALLTYPE Release()
    {
        ULONG ulRef = InterlockedDecrement(&_cRef);

        if (ulRef == 0) {
            delete this;
        }

        return ulRef;
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, VOID** ppvInterface)
    {
        if (IID_IUnknown == riid) {
            AddRef();

            *ppvInterface = (IUnknown*)this;
        }
        else if (__uuidof(IMMNotificationClient) == riid) {
            AddRef();

            *ppvInterface = (IMMNotificationClient*)this;
        }
        else {
            *ppvInterface = NULL;

            return E_NOINTERFACE;
        }

        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDeviceId)
    {
        IMMDevice* pDevice = nullptr;
        IPropertyStore* pStore = nullptr;
        PROPVARIANT varName;
        DeviceType type;

        if (lstrcmpW(spDeviceId, pwstrDeviceId) != 0 && pOnDefaultDeviceChange != nullptr) {
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

            pOnDefaultDeviceChange(std::string(CW2A(varName.pwszVal)), type);

            spDeviceId = _wcsdup(pwstrDeviceId);

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

    HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR pwstrDeviceId)
    {
        IMMDevice* pDevice = nullptr;
        IPropertyStore* pStore = nullptr;
        PROPVARIANT varName;
        std::string deviceName;
        IMMEndpoint* pEndpoint = nullptr;
        EDataFlow flow;
        DeviceType type;

        if (pOnDeviceAdd != nullptr) {
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

            pOnDeviceAdd(deviceName, type);

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

    HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR pwstrDeviceId)
    {
        IMMDevice* pDevice = nullptr;
        IPropertyStore* pStore = nullptr;
        PROPVARIANT varName;
        std::string deviceName;
        IMMEndpoint* pEndpoint = nullptr;
        EDataFlow flow;
        DeviceType type;

        if (pOnDeviceRemove != nullptr) {
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

            pOnDeviceRemove(deviceName, type);

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

    HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState)
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
            if (pOnDeviceRemove != nullptr) {
                pOnDeviceRemove(deviceName, type);
            }

            break;

        case DEVICE_STATE_ACTIVE:
            if (pOnDeviceAdd != nullptr) {
                pOnDeviceAdd(deviceName, type);
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

    HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key)
    {
        return S_OK;
    }
};

static CMMNotificationClient sClient;

namespace BriskAudio {
void (*pOnDefaultDeviceChange)(std::string aDeviceName, DeviceType aType) = nullptr;
void (*pOnDeviceAdd)(std::string aDeviceName, DeviceType aType) = nullptr;
void (*pOnDeviceRemove)(std::string aDeviceName, DeviceType aType) = nullptr;

Device::~Device()
{
    if (nativeHandle != nullptr) {
        ((IMMDevice*)nativeHandle)->Release();
    }
}

Exit init()
{
    if (FAILED(CoInitialize(nullptr))) {
        return Exit::FAILURE;
    }

    if (FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&spEnumerator))) {
        CoUninitialize();

        return Exit::FAILURE;
    }

    if (FAILED(spEnumerator->RegisterEndpointNotificationCallback(&sClient))) {
        CoUninitialize();

        return Exit::FAILURE;
    }

    return Exit::SUCCESS;
}

Exit getDeviceCount(DeviceType aType, unsigned int& aCount)
{
    EDataFlow flow = (aType == DeviceType::PLAYBACK) ? eRender : eCapture;
    IMMDeviceCollection* pCollection = nullptr;

    if (FAILED(spEnumerator->EnumAudioEndpoints(flow, DEVICE_STATE_ACTIVE, &pCollection))) {
        aCount = 0;

        return Exit::FAILURE;
    }

    if (FAILED(pCollection->GetCount(&aCount))) {
        pCollection->Release();

        aCount = 0;

        return Exit::FAILURE;
    }

    pCollection->Release();

    return Exit::SUCCESS;
}

Exit getDefaultDevice(DeviceType aType, Device& appDevice)
{
    EDataFlow flow = (aType == DeviceType::PLAYBACK) ? eRender : eCapture;
    IPropertyStore* pStore = nullptr;
    PROPVARIANT varName;

    if (FAILED(spEnumerator->GetDefaultAudioEndpoint(flow, eConsole, (IMMDevice**)&appDevice.nativeHandle))) {
        return Exit::FAILURE;
    }

    if (FAILED(((IMMDevice*)appDevice.nativeHandle)->OpenPropertyStore(STGM_READ, &pStore))) {
        return Exit::FAILURE;
    }

    if (FAILED(pStore->GetValue(PKEY_Device_FriendlyName, &varName))) {
        pStore->Release();

        return Exit::FAILURE;
    }

    appDevice.name = CW2A(varName.pwszVal);
    appDevice.type = aType;

    if (FAILED(PropVariantClear(&varName))) {
        pStore->Release();

        return Exit::FAILURE;
    }

    pStore->Release();

    return Exit::SUCCESS;
}

Exit getDevice(DeviceType aType, unsigned int aIndex, Device& appDevice)
{
    EDataFlow flow = (aType == DeviceType::PLAYBACK) ? eRender : eCapture;
    IMMDeviceCollection* pCollection = nullptr;
    unsigned int count;
    IPropertyStore* pStore = nullptr;
    PROPVARIANT varName;

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

    if (FAILED(pCollection->Item(aIndex, (IMMDevice**)&appDevice.nativeHandle))) {
        pCollection->Release();

        return Exit::FAILURE;
    }

    if (FAILED(((IMMDevice*)appDevice.nativeHandle)->OpenPropertyStore(STGM_READ, &pStore))) {
        pCollection->Release();

        return Exit::FAILURE;
    }

    if (FAILED(pStore->GetValue(PKEY_Device_FriendlyName, &varName))) {
        pStore->Release();

        pCollection->Release();

        return Exit::FAILURE;
    }

    appDevice.name = CW2A(varName.pwszVal);
    appDevice.type = aType;

    if (FAILED(PropVariantClear(&varName))) {
        pStore->Release();

        pCollection->Release();

        return Exit::FAILURE;
    }

    pStore->Release();
    pCollection->Release();

    return Exit::SUCCESS;
}

Exit quit()
{
    if (spDeviceId != nullptr) {
        CoTaskMemFree((void*)spDeviceId);

        spDeviceId = nullptr;
    }

    if (FAILED(spEnumerator->UnregisterEndpointNotificationCallback(&sClient))) {
        return Exit::FAILURE;
    }

    spEnumerator->Release();

    spEnumerator = nullptr;

    CoUninitialize();

    return Exit::SUCCESS;
}
}
#endif
