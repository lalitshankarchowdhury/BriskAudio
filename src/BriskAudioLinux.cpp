#ifdef __linux__
#include "BriskAudio.hpp"

#include <alsa/asoundlib.h>

static char** hints;

namespace BriskAudio {
    unsigned int DeviceInfoCollection::getDeviceCount(DeviceType deviceType) {
        if (snd_device_name_hint(-1, "pcm", (void***) &hints) != 0) {
            return 0;
        }

        unsigned int deviceCount = 0;

        for (char** n = hints; *n != NULL; n++) {
            deviceCount++;
        }

        return deviceCount;
    }

    DeviceInfo DeviceInfoCollection::getDeviceInfo(unsigned int i, DeviceType deviceType) {
        DeviceInfo temp;

        char* temp_name;
        char* temp_description;

        temp_name = snd_device_name_get_hint(hints[i], "NAME");
        temp_description = snd_device_name_get_hint(hints[i], "DESC");

        if (temp_name == NULL || temp_description == NULL) {
            return DeviceInfo();
        }

        temp.name = temp_name;
        temp.description = temp_description;

        temp.isValid = true;

        return temp;
    }
}
#endif
