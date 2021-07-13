#pragma once

#include <string>
#include <set>

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
    unsigned int defaultSampleRate;
    std::set<unsigned int> supportedNumChannels;
    std::set<unsigned int> supportedSampleRates;

    Device()
    {
        name = "??????";
        type = DeviceType::PLAYBACK;
        defaultSampleRate = 0;
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
