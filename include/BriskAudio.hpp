#pragma once

#include <string>

namespace BA {
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

    Device()
    {
        name = "??????";
        type = DeviceType::PLAYBACK;
        nativeHandle_ = nullptr;
    }

    ~Device(); // Redefined in platform specific files

private:
    void* nativeHandle_;

    friend struct DeviceEnumerator;
};

struct DeviceEnumerator {
    DeviceType type;

    DeviceEnumerator(DeviceType aType)
    {
        type = aType;
    }

    ~DeviceEnumerator();

    unsigned int returnDeviceCount();
    Device* returnDefaultDevice();
    Device* returnDevice(unsigned int aIndex);
};

extern void (*pOnDefaultDeviceChange)(std::string aDeviceName, DeviceType aType);
extern void (*pOnDeviceAdd)(std::string aDeviceName, DeviceType aType);
extern void (*pOnDeviceRemove)(std::string aDeviceName, DeviceType aType);

Exit init();
Exit quit();
}
