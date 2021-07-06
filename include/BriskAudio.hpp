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
unsigned int getDeviceCount(DeviceType aType);
Exit getDefaultDevice(DeviceType aType, Device** appDevice);
Exit getDevice(unsigned int aIndex, DeviceType aType, Device** appDevice);
Exit registerDeviceEventCallbacks(void (*apOnDefaultDeviceChange)(std::string aDeviceName, DeviceType aType));
Exit quit();
}
