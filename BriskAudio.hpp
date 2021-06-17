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

    struct Limits {
        unsigned int min;
        unsigned int max;
    };

    struct DeviceInfo {
        std::string name;
        std::string ID;
        DeviceType deviceType;
        Limits inputChannels;
        Limits outputChannels;
        Limits duplexChannels;
        std::vector<unsigned int> supportedSampleRates;
        Limits bufferSizeInFrames;
        BufferFormat supportedBufferFormats;

        DeviceInfo() {
            inputChannels = Limits {0, 0};
            outputChannels = Limits {0, 0};
            duplexChannels = Limits {0, 0};
            bufferSizeInFrames = Limits {0, 0};
        }

        unsigned int getDeviceCount();
        DeviceInfo getDeviceInfo(unsigned int i);
    };
    
    struct Device {
        void* nativeDeviceHandle;
        DeviceInfo info;
    }
}
