#include <vector>
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

    struct DeviceInfo {
        bool isValid = false;
        std::string name;
        std::string description;
        DeviceType type;
    };

    struct DeviceInfoCollection {
        unsigned int getDeviceCount(DeviceType aType);
        DeviceInfo getDeviceInfo(unsigned int aIndex, DeviceType aType);
    };

    Exit init();
    void quit();
}
