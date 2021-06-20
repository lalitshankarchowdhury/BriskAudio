#include <vector>
#include <string>

namespace BriskAudio {
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
        PLAYBACK,
    };

    struct Limits {
        unsigned int min;
        unsigned int max;
    };

    struct DeviceInfo {
        bool isValid;
        std::string name;
        std::string description;
        DeviceType deviceType;
        Limits inputChannels;
        Limits outputChannels;
        Limits duplexChannels;
        std::vector<unsigned int> supportedSampleRates;
        Limits bufferSizeInFrames;
        BufferFormat supportedBufferFormats;

        DeviceInfo() {
            isValid = false;
        }
    };

    struct DeviceEnumerator {
        unsigned int getDeviceCount(DeviceType deviceType);
        DeviceInfo getDeviceInfo(unsigned int i, DeviceType deviceType);
    };
    
    struct Device {
        void* nativeDeviceHandle;
        DeviceInfo info;
    };
}
