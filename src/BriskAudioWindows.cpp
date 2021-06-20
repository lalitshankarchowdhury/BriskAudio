#ifdef _WIN32
#include "BriskAudio.hpp"

#include <atlstr.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>

static IMMDeviceCollection *collection = NULL;

namespace BriskAudio
{
	unsigned int DeviceEnumerator::getDeviceCount()
	{
		HRESULT result;
		IMMDeviceEnumerator *enumerator = NULL;

		CoInitialize(NULL);

		result = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void **)&enumerator);

		if (FAILED(result))
		{
			return 0;
		}

		result = enumerator->EnumAudioEndpoints(eRender, DEVICE_STATEMASK_ALL, &collection);

		if (FAILED(result))
		{
			return 0;
		}

		unsigned int deviceCount = 0;

		result = collection->GetCount(&deviceCount);

		if (FAILED(result))
		{
			return 0;
		}

		return deviceCount;
	}

	DeviceInfo DeviceEnumerator::getDeviceInfo(unsigned int i)
	{
		HRESULT result;
		IMMDevice *device = NULL;
		DeviceInfo temp;
		IPropertyStore *store = NULL;
		PROPVARIANT varName;

		result = collection->Item(i, &device);

		if (FAILED(result))
		{
			return DeviceInfo();
		}

		result = device->OpenPropertyStore(STGM_READ, &store);

		if (FAILED(result))
		{
			return DeviceInfo();
		}

		PropVariantInit(&varName);

		result = store->GetValue(PKEY_Device_FriendlyName, &varName);

		if (FAILED(result))
		{
			return DeviceInfo();
		}

		temp.name = CW2A(varName.pwszVal);

		result = store->GetValue(PKEY_Device_DeviceDesc, &varName);

		if (FAILED(result))
		{
			return DeviceInfo();
		}

		temp.description = CW2A(varName.pwszVal);

		CoUninitialize();

		temp.isValid = true;

		return temp;
	}
}
#endif
