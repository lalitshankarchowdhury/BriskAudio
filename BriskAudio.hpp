#include <vector>
#include <string>

namespace BriskAudio {
    enum ExitStatus {
        SUCCESS,
        FAILURE
    };

    enum class BufferFormat {
        U_INT_8,
        S_INT_8,
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

    struct ChannelCount {
        unsigned int min;
        unsigned int max;     
    };

    struct DeviceInfo {
        std::string name;
        std::string ID;
        bool isDefault;
        DeviceType deviceType;
        ChannelCount inputChannels;
        ChannelCount outputChannels;
        ChannelCount duplexChannels;
        std::vector<unsigned int> supportedSampleRates;
        std::vector<unsigned int> supportedBufferSizes;
        BufferFormat supportedBufferFormats;

        DeviceInfo() {
            isDefault = false;
            inputChannels = ChannelCount {0, 0};
            outputChannels = ChannelCount {0, 0};
            duplexChannels = ChannelCount {0, 0};
        }

        unsigned int getDeviceCount();
        DeviceInfo getDeviceInfo(unsigned int i);
    };
}
