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
        name = "??????";
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

    DeviceEnumerator()
    {
        type = DeviceType::PLAYBACK;
    }

    ~DeviceEnumerator() {}

    unsigned int returnDeviceCount();
    Device* returnDefaultDevice();
    Device* returnDevice(unsigned int aIndex);
};

struct DeviceEventNotifier;

Exit init();
Exit quit();
}
