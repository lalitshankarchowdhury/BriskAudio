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

    struct DeviceInfo {
        bool isValid = false;
        std::string name;
        std::string description;
        DeviceType type;
        unsigned int numChannels;
        unsigned long defaultSampleRate;
    };

    struct DeviceInfoCollection {
        unsigned int getDeviceCount(DeviceType aType);
        DeviceInfo getDeviceInfo(unsigned int aIndex, DeviceType aType);
    };

    Exit init();
    void quit();
}
