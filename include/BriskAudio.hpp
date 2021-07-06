#pragma once

#include <string>

namespace BriskAudio {
enum class Exit {
    SUCCESS,
    FAILURE
};

enum class DeviceType {
    PLAYBACK,
    CAPTURE
};

struct Device {
    std::string name;
    DeviceType type;
    void* nativeHandle;

    Device()
    {
        name = "INVALID";
        type = DeviceType::PLAYBACK;
        nativeHandle = nullptr;
    }

    ~Device();
};

Exit init();
Exit getDeviceCount(DeviceType aType, unsigned int* aCount);
Exit getDefaultDevice(DeviceType aType, Device** appDevice);
Exit getDevice(DeviceType aType, unsigned int aIndex, Device** appDevice);
Exit registerDeviceEventCallbacks(
    void (*apOnDefaultDeviceChange)(std::string aDeviceName, DeviceType aType),
    void (*apOnDeviceAdd)(std::string aDeviceName, DeviceType aType),
    void (*apOnDeviceRemove)(std::string aDeviceName, DeviceType aType));
Exit quit();
}
