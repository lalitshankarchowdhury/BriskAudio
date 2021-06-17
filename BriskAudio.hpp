#include <vector>
#include <string>

namespace BriskAudio {
    enum ExitStatus {
        SUCCESS,
        FAILURE
    };

    enum class BufferFormat {
        U_INT_8,
        S_INT_16,
        FLOAT_32,
        FLOAT_64
    };

    enum class DeviceType {
        CAPTURE,
        PLAYBACK
    };

    struct DeviceInfo {
        std::string name;
        std::string ID;
        bool isDefault;
        DeviceType deviceType;
        unsigned int minInputChannels;
        unsigned int minOutputChannels;
        unsigned int maxInputChannels;
        unsigned int maxOutputChannels;
        std::vector<unsigned int> supportedSampleRates;
        std::vector<unsigned int> supportedBufferSizes;
        BufferFormat supportedBufferFormats;

        DeviceInfo() {
            isDefault = false;
            minInputChannels = 0;
            minOutputChannels = 0;
            maxInputChannels = 0;
            maxOutputChannels = 0;
        }

        unsigned int getDeviceCount();
        DeviceInfo getDeviceInfo(unsigned int i);
    };
}
