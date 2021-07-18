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
    U_INT_8 = 1,
    S_INT_16 = 2,
    S_INT_24 = 4,
    S_INT_32 = 8,
    FLOAT_32 = 16,
    FLOAT_64 = 32
};

constexpr BufferFormat operator&(BufferFormat left, BufferFormat right)
{
    return static_cast<BufferFormat>(static_cast<unsigned int>(left) & static_cast<unsigned int>(right));
}

constexpr BufferFormat operator|(BufferFormat left, BufferFormat right)
{
    return static_cast<BufferFormat>(static_cast<unsigned int>(left) | static_cast<unsigned int>(right));
}

constexpr BufferFormat operator|=(BufferFormat& left, BufferFormat right)
{
    left = static_cast<BufferFormat>(static_cast<unsigned int>(left) | static_cast<unsigned int>(right));

    return left;
}

// NativeDeviceHandle is defined in platform-specific headers
struct Device : public NativeDeviceHandle {
    std::string name;
    DeviceType type;
    std::vector<unsigned int> supportedChannels;
    std::vector<unsigned int> sampleRates;
    BufferFormat supportedFormats;

    Device()
    {
        name = "??????";
        type = DeviceType::PLAYBACK;
        supportedFormats = static_cast<BufferFormat>(0);
    }

    Exit getVolume(float& arVolume);
    Exit setVolume(float aVolume);
    bool isStreamFormatSupported(unsigned int aNumChannels, unsigned int aSampleRate, BufferFormat aFormat);
};

Exit init();
Exit getDeviceCount(unsigned int& arDeviceCount, DeviceType aType);
Exit openDefaultDevice(Device& arDevice, DeviceType aType);
Exit openDevice(Device& arDevice, unsigned int aIndex, DeviceType aType);
Exit openDevice(Device& arDevice, std::string aDeviceName);
Exit closeDevice(Device& arDevice);
Exit quit();
}
