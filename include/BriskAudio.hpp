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

extern void (*pOnDefaultDeviceChange)(std::string aDeviceName, DeviceType aType);
extern void (*pOnDeviceAdd)(std::string aDeviceName, DeviceType aType);
extern void (*pOnDeviceRemove)(std::string aDeviceName, DeviceType aType);

Exit init();
Exit getDeviceCount(DeviceType aType, unsigned int& aCount);
Exit getDefaultDevice(DeviceType aType, Device& appDevice);
Exit getDevice(unsigned int aIndex, DeviceType aType, Device& appDevice);
Exit quit();
}
