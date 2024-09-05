#pragma once

#ifdef _WIN32
#include <Audioclient.h>
#include <Endpointvolume.h>
#include <atlbase.h>
#include <initguid.h>
#include <mmdeviceapi.h>
#include <string>

namespace BriskAudio {
	struct NativeStreamHandle {
		CComPtr<IAudioRenderClient> pRenderClient;
		CComPtr<IAudioCaptureClient> pCaptureClient;

		NativeStreamHandle();
	};

	struct NativeDeviceHandle : public IAudioEndpointVolumeCallback, public IMMNotificationClient {
		CComPtr<IMMDevice> pDevice;
		CComPtr<IAudioClient> pClient;
		CComPtr<IAudioEndpointVolume> pVolume;
		GUID eventContext;
		void (*pOnVolumeChange)(std::string aDeviceName, float aVolume);
		void (*pOnMute)(std::string aDeviceName);
		void (*pOnDefaultDeviceChange)(std::string aDeviceName);
		void (*pOnAnyDeviceAdd)(std::string aDeviceName);
		void (*pOnAnyDeviceRemove)(std::string aDeviceName);
		void (*pOnAnyDeviceStateChange)(std::string aDeviceName, std::string state);

		NativeDeviceHandle();
		virtual ~NativeDeviceHandle();
		ULONG STDMETHODCALLTYPE AddRef();
		ULONG STDMETHODCALLTYPE Release();
		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, VOID** ppvInterface);
		HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDeviceId);
		HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR pwstrDeviceId);
		HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR pwstrDeviceId);
		HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState);
		HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key);
		HRESULT STDMETHODCALLTYPE OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify);

	private:
		LONG referenceCount_;
	};
}
#endif
