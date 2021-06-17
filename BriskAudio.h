#include <vector>
#include <string>

#include "BriskAudioLinux.h"
#include "BriskAudioWindows.h"

namespace BriskAudio {
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
        DeviceType deviceType;
        unsigned int maxInputChannels;
        unsigned int maxOutputChannels;
        std::vector<unsigned int> sampleRates;
        std::vector<unsigned int> supportedBufferSizes;
        BufferFormat supportedBufferFormats;
    };
}