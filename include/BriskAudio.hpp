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

enum class BufferFormat {
    U_INT_8,
    S_INT_16,
    S_INT_24,
    S_INT_32,
    FLOAT_32,
    FLOAT_64
};

struct Device {
    std::string name;
    DeviceType type;

    Device()
    {
        name = "??????";
        type = DeviceType::PLAYBACK;
    }

    ~Device();

    bool isStreamFormatSupported(BufferFormat aFormat, unsigned int aNumChannels, unsigned int aSampleRate);

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
