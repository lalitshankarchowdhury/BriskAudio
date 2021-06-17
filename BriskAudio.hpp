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
        S_INT_24,
        S_INT_32,
        FLOAT_32,
        FLOAT_64
    };

    enum class DeviceType {
        CAPTURE,
        PLAYBACK
    };

    struct ChannelCountLimit {
        unsigned int min;
        unsigned int max;     
    };

    struct BufferSizeLimit {
        unsigned int min;
        unsigned int max;
    };

    struct DeviceInfo {
        std::string name;
        std::string ID;
        DeviceType deviceType;
        ChannelCountLimit inputChannels;
        ChannelCountLimit outputChannels;
        ChannelCountLimit duplexChannels;
        std::vector<unsigned int> supportedSampleRates;
        BufferSizeLimit bufferSizes;
        BufferFormat supportedBufferFormats;

        DeviceInfo() {
            inputChannels = ChannelCount {0, 0};
            outputChannels = ChannelCount {0, 0};
            duplexChannels = ChannelCount {0, 0};
        }

        unsigned int getDeviceCount();
        DeviceInfo getDeviceInfo(unsigned int i);
    };
}
