#ifdef _WIN32
#include "BriskAudioWindows.hpp"
#include <atlstr.h>
#include <functiondiscoverykeys_devpkey.h>

static bool sIsCoInitialized = false;
static IMMDeviceEnumerator* spEnumerator = nullptr;

namespace BriskAudio {
Device::~Device()
{
    if (nativeHandle_ != nullptr) {
        ((IMMDevice*)nativeHandle_)->Release();
    }
}

unsigned int DeviceEnumerator::returnDeviceCount()
{
    EDataFlow flow = (type == DeviceType::PLAYBACK) ? eRender : eCapture;
    IMMDeviceCollection* pCollection = nullptr;
    unsigned int count = 0;

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

Device* DeviceEnumerator::returnDefaultDevice()
{
    EDataFlow flow = (type == DeviceType::PLAYBACK) ? eRender : eCapture;
    IPropertyStore* pStore = nullptr;
    PROPVARIANT varName;
    Device* pDevice = new Device();

    if (FAILED(spEnumerator->GetDefaultAudioEndpoint(flow, eConsole, (IMMDevice**)&pDevice->nativeHandle_))) {
        delete pDevice;

        return nullptr;
    }

    if (FAILED(((IMMDevice*)pDevice->nativeHandle_)->OpenPropertyStore(STGM_READ, &pStore))) {
        delete pDevice;

        return nullptr;
    }

    if (FAILED(pStore->GetValue(PKEY_Device_FriendlyName, &varName))) {
        delete pDevice;

        pStore->Release();

        return nullptr;
    }

    pDevice->name = CW2A(varName.pwszVal);
    pDevice->type = type;

    if (FAILED(PropVariantClear(&varName))) {
        delete pDevice;

        pStore->Release();

        return nullptr;
    }

    pStore->Release();

    return pDevice;
}

Device* DeviceEnumerator::returnDevice(unsigned int aIndex)
{
    EDataFlow flow = (type == DeviceType::PLAYBACK) ? eRender : eCapture;
    IMMDeviceCollection* pCollection = nullptr;
    unsigned int count;
    IPropertyStore* pStore = nullptr;
    PROPVARIANT varName;
    Device* pDevice = new Device();

    if (FAILED(spEnumerator->EnumAudioEndpoints(flow, DEVICE_STATE_ACTIVE, &pCollection))) {
        delete pDevice;

        return nullptr;
    }

    if (FAILED(pCollection->GetCount(&count))) {
        delete pDevice;

        pCollection->Release();

        return nullptr;
    }

    if (aIndex >= count) {
        delete pDevice;

        pCollection->Release();

        return nullptr;
    }

    if (FAILED(pCollection->Item(aIndex, (IMMDevice**)&pDevice->nativeHandle_))) {
        delete pDevice;

        pCollection->Release();

        return nullptr;
    }

    if (FAILED(((IMMDevice*)pDevice->nativeHandle_)->OpenPropertyStore(STGM_READ, &pStore))) {
        delete pDevice;

        pCollection->Release();

        return nullptr;
    }

    if (FAILED(pStore->GetValue(PKEY_Device_FriendlyName, &varName))) {
        pStore->Release();

        pCollection->Release();

        return nullptr;
    }

    pDevice->name = CW2A(varName.pwszVal);
    pDevice->type = type;

    if (FAILED(PropVariantClear(&varName))) {
        pStore->Release();

        pCollection->Release();

        return nullptr;
    }

    pStore->Release();
    pCollection->Release();

    return pDevice;
}

DeviceEventNotifier::DeviceEventNotifier()
{
    cRef_ = 1;
    pDeviceId_ = _wcsdup(L" ");
    pOnDefaultDeviceChange_ = nullptr;
    pOnDeviceAdd_ = nullptr;
    pOnDeviceRemove_ = nullptr;
}

DeviceEventNotifier::~DeviceEventNotifier()
{
    CoTaskMemFree((void*)pDeviceId_);
}

ULONG STDMETHODCALLTYPE DeviceEventNotifier::AddRef()
{
    return InterlockedIncrement(&cRef_);
}

ULONG STDMETHODCALLTYPE DeviceEventNotifier::Release()
{
    ULONG ulRef = InterlockedDecrement(&cRef_);

    if (ulRef == 0) {
        delete this;
    }

    return ulRef;
}

HRESULT STDMETHODCALLTYPE DeviceEventNotifier::QueryInterface(REFIID riid, VOID** ppvInterface)
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

HRESULT STDMETHODCALLTYPE DeviceEventNotifier::OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDeviceId)
{
    IMMDevice* pDevice = nullptr;
    IPropertyStore* pStore = nullptr;
    PROPVARIANT varName;
    DeviceType type;

    if (lstrcmpW(pDeviceId_, pwstrDeviceId) != 0 && pOnDefaultDeviceChange_ != nullptr) {
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

        pOnDefaultDeviceChange_(std::string(CW2A(varName.pwszVal)), type);

        pDeviceId_ = _wcsdup(pwstrDeviceId);

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

HRESULT STDMETHODCALLTYPE DeviceEventNotifier::OnDeviceAdded(LPCWSTR pwstrDeviceId)
{
    IMMDevice* pDevice = nullptr;
    IPropertyStore* pStore = nullptr;
    PROPVARIANT varName;
    std::string deviceName;
    IMMEndpoint* pEndpoint = nullptr;
    EDataFlow flow;
    DeviceType type;

    if (pOnDeviceAdd_ != nullptr) {
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

        pOnDeviceAdd_(deviceName, type);

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

HRESULT STDMETHODCALLTYPE DeviceEventNotifier::OnDeviceRemoved(LPCWSTR pwstrDeviceId)
{
    IMMDevice* pDevice = nullptr;
    IPropertyStore* pStore = nullptr;
    PROPVARIANT varName;
    std::string deviceName;
    IMMEndpoint* pEndpoint = nullptr;
    EDataFlow flow;
    DeviceType type;

    if (pOnDeviceRemove_ != nullptr) {
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

        pOnDeviceRemove_(deviceName, type);

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

HRESULT STDMETHODCALLTYPE DeviceEventNotifier::OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState)
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
        if (pOnDeviceRemove_ != nullptr) {
            pOnDeviceRemove_(deviceName, type);
        }

        break;

    case DEVICE_STATE_ACTIVE:
        if (pOnDeviceAdd_ != nullptr) {
            pOnDeviceAdd_(deviceName, type);
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

HRESULT STDMETHODCALLTYPE DeviceEventNotifier::OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key)
{
    return S_OK;
}

Exit DeviceEventNotifier::registerEventCallbacks(
    void (*apOnDefaultDeviceChange)(std::string aDeviceName, DeviceType aType),
    void (*apOnDeviceAdd)(std::string aDeviceName, DeviceType aType),
    void (*apOnDeviceRemove)(std::string aDeviceName, DeviceType aType))
{
    pOnDefaultDeviceChange_ = apOnDefaultDeviceChange;
    pOnDeviceAdd_ = apOnDeviceAdd;
    pOnDeviceRemove_ = apOnDeviceRemove;

    if (FAILED(spEnumerator->RegisterEndpointNotificationCallback(this))) {
        return Exit::FAILURE;
    }

    return Exit::SUCCESS;
}

Exit DeviceEventNotifier::unregisterEventCallbacks()
{
    pOnDefaultDeviceChange_ = nullptr;
    pOnDeviceAdd_ = nullptr;
    pOnDeviceRemove_ = nullptr;

    if (FAILED(spEnumerator->UnregisterEndpointNotificationCallback(this))) {
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

    // CoUninitialize() must be called only once when the program exits
    atexit((void(__cdecl*)())CoUninitialize);

    if (FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&spEnumerator))) {
        CoUninitialize();

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

    spEnumerator->Release();
    spEnumerator = nullptr;

    return Exit::SUCCESS;
}
}
#endif
