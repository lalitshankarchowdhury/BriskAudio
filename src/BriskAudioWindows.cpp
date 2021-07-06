#ifdef _WIN32
#include "../include/BriskAudio.hpp"
#include <atlstr.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>

static HANDLE shConsoleHandle = nullptr;
static DWORD sDefaultConsoleMode;
static IMMDeviceEnumerator* spEnumerator = nullptr;
static LPCWSTR spDeviceId = nullptr;

namespace BriskAudio {
Device::~Device()
{
    if (nativeHandle != nullptr) {
        ((IMMDevice*)nativeHandle)->Release();
    }
}

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

Exit getDeviceCount(DeviceType aType, unsigned int* aCount)
{
    if (aCount == nullptr) {
        return Exit::FAILURE;
    }

    EDataFlow flow = (aType == DeviceType::PLAYBACK) ? eRender : eCapture;
    IMMDeviceCollection* pCollection = nullptr;

    if (FAILED(spEnumerator->EnumAudioEndpoints(flow, DEVICE_STATE_ACTIVE, &pCollection))) {
        *aCount = 0;
       
        return Exit::FAILURE;
    }

    if (FAILED(pCollection->GetCount(aCount))) {
        pCollection->Release();

        *aCount = 0;
       
        return Exit::FAILURE;
    }

    pCollection->Release();

    return Exit::SUCCESS;
}

Exit getDefaultDevice(DeviceType aType, Device* appDevice)
{
    if (appDevice == nullptr) {
        return Exit::FAILURE;
    }

    EDataFlow flow = (aType == DeviceType::PLAYBACK) ? eRender : eCapture;
    IPropertyStore* pStore = nullptr;
    PROPVARIANT varName;

    if (FAILED(spEnumerator->GetDefaultAudioEndpoint(flow, eConsole, (IMMDevice**)&(appDevice)->nativeHandle))) {
         return Exit::FAILURE;
    }

    if (FAILED(((IMMDevice*)(appDevice)->nativeHandle)->OpenPropertyStore(STGM_READ, &pStore))) {
        return Exit::FAILURE;
    }

    if (FAILED(pStore->GetValue(PKEY_Device_FriendlyName, &varName))) {
        pStore->Release();

        return Exit::FAILURE;
    }

    (appDevice)->name = CW2A(varName.pwszVal);
    (appDevice)->type = aType;

    if (FAILED(PropVariantClear(&varName))) {
        pStore->Release();

        return Exit::FAILURE;
    }

    pStore->Release();

    return Exit::SUCCESS;
}

Exit getDevice(DeviceType aType, unsigned int aIndex, Device* appDevice)
{
    if (appDevice == nullptr) {
        return Exit::FAILURE;
    }

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

    if (FAILED(pCollection->Item(aIndex, (IMMDevice**)&(appDevice)->nativeHandle))) {
        pCollection->Release();

        return Exit::FAILURE;
    }

    if (FAILED(((IMMDevice*)(appDevice)->nativeHandle)->OpenPropertyStore(STGM_READ, &pStore))) {
        pCollection->Release();

        return Exit::FAILURE;
    }

    if (FAILED(pStore->GetValue(PKEY_Device_FriendlyName, &varName))) {
        pStore->Release();

        pCollection->Release();

        return Exit::FAILURE;
    }

    (appDevice)->name = CW2A(varName.pwszVal);
    (appDevice)->type = aType;

    if (FAILED(PropVariantClear(&varName))) {
        pStore->Release();

        pCollection->Release();

        return Exit::FAILURE;
    }

    pStore->Release();
    pCollection->Release();

    return Exit::SUCCESS;
}

class CMMNotificationClient : public IMMNotificationClient {
    LONG _cRef;

public:
    CMMNotificationClient()
    {
        _cRef = 1;

        pOnDefaultDeviceChange_ = nullptr;
        pOnDeviceAdd_ = nullptr;
        pOnDeviceRemove_ = nullptr;
    }

    CMMNotificationClient(void (*apOnDefaultDeviceChange)(std::string aDeviceName, DeviceType aType), void (*apOnDeviceAdd)(std::string aDeviceName, DeviceType aType), void (*apOnDeviceRemove)(std::string aDeviceName, DeviceType aType))
    {
        _cRef = 1;

        pOnDefaultDeviceChange_ = apOnDefaultDeviceChange;
        pOnDeviceAdd_ = apOnDeviceAdd;
        pOnDeviceRemove_ = apOnDeviceRemove;
    }

    ~CMMNotificationClient()
    {
        pOnDefaultDeviceChange_ = nullptr;
        pOnDeviceAdd_ = nullptr;
        pOnDeviceRemove_ = nullptr;
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

        if (lstrcmpW(spDeviceId, pwstrDeviceId) != 0 && pOnDefaultDeviceChange_ != nullptr) {
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

            pOnDefaultDeviceChange_(std::string(CW2A(varName.pwszVal)), (flow == eRender) ? DeviceType::PLAYBACK : DeviceType::CAPTURE);

            spDeviceId = wcsdup(pwstrDeviceId);

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
        if (pOnDeviceAdd_ != nullptr) {
            pOnDeviceAdd_("Hi!", DeviceType::PLAYBACK);
        }

        return S_OK;
    };

    HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR pwstrDeviceId)
    {
        if (pOnDeviceRemove_ != nullptr) {
            pOnDeviceRemove_("Hi!", DeviceType::PLAYBACK);
        }

        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState)
    {
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key)
    {
        return S_OK;
    }

private:
    void (*pOnDefaultDeviceChange_)(std::string aDeviceName, DeviceType aType);
    void (*pOnDeviceAdd_)(std::string aDeviceName, DeviceType aType);
    void (*pOnDeviceRemove_)(std::string aDeviceName, DeviceType aType);
};

static CMMNotificationClient* spClient = nullptr;

Exit registerDeviceEventCallbacks(void (*apOnDefaultDeviceChange)(std::string aDeviceName, DeviceType aType), void (*apOnDeviceAdd)(std::string aDeviceName, DeviceType aType), void (*apOnDeviceRemove)(std::string aDeviceName, DeviceType aType))
{
    spClient = new CMMNotificationClient(apOnDefaultDeviceChange, apOnDeviceAdd, apOnDeviceRemove);

    if (FAILED(spEnumerator->RegisterEndpointNotificationCallback(spClient))) {
        return Exit::FAILURE;
    }

    return Exit::SUCCESS;
}

Exit quit()
{
    if (spDeviceId != nullptr) {
        CoTaskMemFree((void*)spDeviceId);

        spDeviceId = nullptr;
    }

    if (spClient != nullptr) {
        if (FAILED(spEnumerator->UnregisterEndpointNotificationCallback(spClient))) {
            return Exit::FAILURE;
        }

        delete spClient;

        spClient = nullptr;
    }

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
