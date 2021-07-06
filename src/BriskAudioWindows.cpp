#ifdef _WIN32
#include "BriskAudio.hpp"
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

unsigned int getDeviceCount(DeviceType aType)
{
    EDataFlow flow = (aType == DeviceType::PLAYBACK) ? eRender : eCapture;
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

Exit getDefaultDevice(DeviceType aType, Device** appDevice)
{
    if (appDevice == nullptr) {
        return Exit::FAILURE;
    }

    EDataFlow flow = (aType == DeviceType::PLAYBACK) ? eRender : eCapture;
    IPropertyStore* pStore = nullptr;
    PROPVARIANT varName;

    *appDevice = new Device();

    if (FAILED(spEnumerator->GetDefaultAudioEndpoint(flow, eConsole, (IMMDevice**)&(*appDevice)->nativeHandle))) {
        *appDevice = nullptr;

        return Exit::FAILURE;
    }

    if (FAILED(((IMMDevice*)(*appDevice)->nativeHandle)->OpenPropertyStore(STGM_READ, &pStore))) {
        delete *appDevice;

        *appDevice = nullptr;

        return Exit::FAILURE;
    }

    if (FAILED(pStore->GetValue(PKEY_Device_FriendlyName, &varName))) {
        pStore->Release();

        delete *appDevice;

        *appDevice = nullptr;

        return Exit::FAILURE;
    }

    (*appDevice)->name = CW2A(varName.pwszVal);
    (*appDevice)->type = aType;

    if (FAILED(PropVariantClear(&varName))) {
        pStore->Release();

        delete *appDevice;

        *appDevice = nullptr;

        return Exit::FAILURE;
    }

    pStore->Release();

    return Exit::SUCCESS;
}

Exit getDevice(unsigned int aIndex, DeviceType aType, Device** appDevice)
{
    if (appDevice == nullptr) {
        return Exit::FAILURE;
    }

    EDataFlow flow = (aType == DeviceType::PLAYBACK) ? eRender : eCapture;
    IMMDeviceCollection* pCollection = nullptr;
    unsigned int count;
    IPropertyStore* pStore = nullptr;
    PROPVARIANT varName;

    *appDevice = new Device();

    if (FAILED(spEnumerator->EnumAudioEndpoints(flow, DEVICE_STATE_ACTIVE, &pCollection))) {
        *appDevice = nullptr;

        return Exit::FAILURE;
    }

    if (FAILED(pCollection->GetCount(&count))) {
        pCollection->Release();

        *appDevice = nullptr;

        return Exit::FAILURE;
    }

    if (aIndex >= count) {
        pCollection->Release();

        *appDevice = nullptr;

        return Exit::FAILURE;
    }

    if (FAILED(pCollection->Item(aIndex, (IMMDevice**)&(*appDevice)->nativeHandle))) {
        pCollection->Release();

        *appDevice = nullptr;

        return Exit::FAILURE;
    }

    if (FAILED(((IMMDevice*)(*appDevice)->nativeHandle)->OpenPropertyStore(STGM_READ, &pStore))) {
        delete *appDevice;

        pCollection->Release();

        *appDevice = nullptr;

        return Exit::FAILURE;
    }

    if (FAILED(pStore->GetValue(PKEY_Device_FriendlyName, &varName))) {
        pStore->Release();

        delete *appDevice;

        pCollection->Release();

        *appDevice = nullptr;

        return Exit::FAILURE;
    }

    (*appDevice)->name = CW2A(varName.pwszVal);
    (*appDevice)->type = aType;

    if (FAILED(PropVariantClear(&varName))) {
        pStore->Release();

        delete *appDevice;

        pCollection->Release();

        *appDevice = nullptr;

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
    }

    CMMNotificationClient(void (*apOnDefaultDeviceChange)(std::string aDeviceName, DeviceType aType))
    {
        _cRef = 1;

        pOnDefaultDeviceChange_ = apOnDefaultDeviceChange;
    }

    ~CMMNotificationClient()
    {
        pOnDefaultDeviceChange_ = nullptr;
    }

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return InterlockedIncrement(&_cRef);
    }

    ULONG STDMETHODCALLTYPE Release()
    {
        ULONG ulRef = InterlockedDecrement(&_cRef);

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

        if (lstrcmpW(spDeviceId, pwstrDeviceId) != 0) {
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

            pOnDefaultDeviceChange_(std::string(CW2A(pwstrDeviceId)), (flow == eRender) ? DeviceType::PLAYBACK : DeviceType::CAPTURE);

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
        return S_OK;
    };

    HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR pwstrDeviceId)
    {
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
};

static CMMNotificationClient* spClient = nullptr;

Exit registerDeviceEventCallbacks(void (*apOnDefaultDeviceChange)(std::string aDeviceName, DeviceType aType))
{
    if (apOnDefaultDeviceChange == nullptr) {
        return Exit::FAILURE;
    }

    spClient = new CMMNotificationClient(apOnDefaultDeviceChange);

    if (FAILED(spEnumerator->RegisterEndpointNotificationCallback(spClient))) {
        return Exit::FAILURE;
    }

    return Exit::SUCCESS;
}

Exit quit()
{
    if (spDeviceId != nullptr) {
        CoTaskMemFree((void*)spDeviceId);
    }

    if (FAILED(spEnumerator->UnregisterEndpointNotificationCallback(spClient))) {
        return Exit::FAILURE;
    }

    delete spClient;

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
