#include "BriskAudio.hpp"
#include <alsa/asoundlib.h>

static char** hints;

namespace BriskAudio {
    unsigned int DeviceInfoCollection::getDeviceCount(DeviceType aType) {
        if (snd_device_name_hint(-1, "pcm", (void***) &hints) != 0) {
            return 0;
        }

        unsigned int count = 0;

        for (char** n = hints; *n != NULL; n++) {
            count++;
        }

        return count;
    }

    DeviceInfo DeviceInfoCollection::getDeviceInfo(unsigned int aIndex, DeviceType aType) {
        DeviceInfo temp;

        return temp;
    }
}
