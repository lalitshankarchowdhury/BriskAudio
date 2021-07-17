#pragma once

#include "../src/BriskAudioWindows.hpp"
#include <vector>

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

// NativeDeviceHandle is defined in platform-specific headers
struct Device : public NativeDeviceHandle {
    std::string name;
    DeviceType type;
    std::vector<unsigned int> supportedChannels;
    std::vector<unsigned int> sampleRates;

    Device()
    {
        name = "??????";
        type = DeviceType::PLAYBACK;
    }

    Exit getVolume(float& arVolume);
    Exit setVolume(float aVolume);
    bool isStreamFormatSupported(BufferFormat aFormat, unsigned int aNumChannels, unsigned int aSampleRate);
};

Exit init();
Exit getDeviceCount(unsigned int& arDeviceCount, DeviceType aType);
Exit openDefaultDevice(Device& arDevice, DeviceType aType);
Exit openDevice(Device& arDevice, unsigned int aIndex, DeviceType aType);
Exit openDevice(Device& arDevice, std::string aDeviceName);
Exit closeDevice(Device& arDevice);
Exit quit();
}
