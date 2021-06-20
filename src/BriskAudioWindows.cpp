#ifdef _WIN32
#include "BriskAudio.hpp"

#include <atlstr.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>

static HRESULT result;
static IMMDeviceEnumerator *enumerator = NULL;
static IMMDeviceCollection *collection = NULL;

namespace BriskAudio {
	unsigned int DeviceInfoCollection::getDeviceCount(DeviceType deviceType) {
		HRESULT result;
		IMMDeviceEnumerator *enumerator = NULL;
		IMMDeviceCollection *collection = NULL;
		EDataFlow dataFlow;

		result = CoInitialize(NULL);

		if (FAILED(result)) {
			return 0;
		}		

		result = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void **)&enumerator);

		if (FAILED(result)) {
			return 0;
		}

		if (deviceType == DeviceType::PLAYBACK) {
			dataFlow = eRender;
		} else if (deviceType == DeviceType::CAPTURE) {
			dataFlow = eCapture;
		} else {
			dataFlow = eAll;
		}

		result = enumerator->EnumAudioEndpoints(dataFlow, DEVICE_STATEMASK_ALL, &collection);

		if (FAILED(result)) {
			return 0;
		}

		unsigned int deviceCount = 0;

		result = collection->GetCount(&deviceCount);

		if (FAILED(result)) {
			return 0;
		}

		CoUninitialize();

		return deviceCount;
	}

	DeviceInfo DeviceInfoCollection::getDeviceInfo(unsigned int i, DeviceType deviceType) {
		IMMDevice *device = NULL;
		DeviceInfo temp;
		IPropertyStore *store = NULL;
		PROPVARIANT varName;
		EDataFlow dataFlow;

		result = CoInitialize(NULL);

		if (FAILED(result)) {
			return DeviceInfo();
		}

		result = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void **)&enumerator);

		if (FAILED(result)) {
			return DeviceInfo();
		}

		if (deviceType == DeviceType::PLAYBACK) {
			dataFlow = eRender;

			temp.deviceType = DeviceType::PLAYBACK;
		} else if (deviceType == DeviceType::CAPTURE) {
			dataFlow = eCapture;

			temp.deviceType = DeviceType::CAPTURE;
		} else {
			dataFlow = eAll;

			temp.deviceType = DeviceType::ALL;
		}

		result = enumerator->EnumAudioEndpoints(dataFlow, DEVICE_STATEMASK_ALL, &collection);

		if (FAILED(result)) {
			return DeviceInfo();
		}

		result = collection->Item(i, &device);

		if (FAILED(result)) {
			return DeviceInfo();
		}

		result = device->OpenPropertyStore(STGM_READ, &store);

		if (FAILED(result)) {
			return DeviceInfo();
		}

		PropVariantInit(&varName);

		result = store->GetValue(PKEY_DeviceInterface_FriendlyName, &varName);

		if (FAILED(result)) {
			return DeviceInfo();
		}

		temp.name = CW2A(varName.pwszVal);

		result = store->GetValue(PKEY_Device_DeviceDesc, &varName);

		if (FAILED(result)) {
			return DeviceInfo();
		}

		temp.description = CW2A(varName.pwszVal);

		CoUninitialize();

		temp.isValid = true;

		return temp;
	}
}
#endif
