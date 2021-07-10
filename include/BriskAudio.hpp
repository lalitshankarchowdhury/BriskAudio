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

    Device()
    {
        name = "INVALID";
        type = DeviceType::PLAYBACK;
        nativeHandle_ = nullptr;
    }

    ~Device();

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

    unsigned int deviceCount();
    Device* giveDefaultDevice();
    Device* giveDevice(unsigned int aIndex);
};

extern void (*pOnDefaultDeviceChange)(std::string aDeviceName, DeviceType aType);
extern void (*pOnDeviceAdd)(std::string aDeviceName, DeviceType aType);
extern void (*pOnDeviceRemove)(std::string aDeviceName, DeviceType aType);

Exit init();
Exit quit();
}
