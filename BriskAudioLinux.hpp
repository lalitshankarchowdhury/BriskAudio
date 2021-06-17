#ifdef __linux__
#include "BriskAudio.hpp"

#include <alsa/asoundlib.h>

namespace BriskAudio {
    unsigned int DeviceInfo::getDeviceCount() {
        return 0;
    }
}
#endif
