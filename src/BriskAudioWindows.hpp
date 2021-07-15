#pragma once

#ifdef _WIN32
#include <Endpointvolume.h>
#include <mmdeviceapi.h>
#include <iostream>

namespace BriskAudio {
struct NativeDeviceHandle {
    IMMDevice* pDevice;
    IAudioEndpointVolume* pVolume;

    NativeDeviceHandle()
    {
        pDevice = nullptr;
        pVolume = nullptr;
    }

    ~NativeDeviceHandle()
    {
        if (pVolume != nullptr) {
            pVolume->Release();
            pVolume = nullptr;
        }

        if (pDevice != nullptr) {
            pDevice->Release();
            pDevice = nullptr;
        }
    }
};
}
#endif
