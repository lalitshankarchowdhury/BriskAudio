#ifdef __linux
#include "BriskAudio.hpp"
#include <alsa/asoundlib.h>
#include <iostream>

static void** hints;

namespace BriskAudio {
    Exit init() {
        if (snd_device_name_hint(-1, "pcm", &hints) < 0) {
            return Exit::FAILURE;
        }

        return Exit::SUCCESS;
    }

    unsigned int DeviceInfoCollection::getDeviceCount(DeviceType aType) {
        unsigned int count = 0;
        char* IOID;

        const char* filter = (aType == DeviceType::PLAYBACK)? "Output" : "Input";
        
        for (void** n = hints; *n != NULL; n++) {
            IOID = snd_device_name_get_hint(*n, "IOID");

            if (IOID != nullptr) {
                if (strcmp(IOID, filter) == 0) {
                    count++;
                }

                free(IOID);
            } else {
                // If IOID cannot be determined
                count++;
            }
        }

        return count;
    }

    DeviceInfo DeviceInfoCollection::getDeviceInfo(unsigned int aIndex, DeviceType aType) {
        unsigned int count = 0;
        char* NAME;
        char* DESC;
        char* IOID;
        DeviceInfo temp;

        temp.type = aType;

        const char* filter = (aType == DeviceType::PLAYBACK)? "Output" : "Input";
        
        for (void** n = hints; *n != NULL; n++) {
            IOID = snd_device_name_get_hint(*n, "IOID");

            if (IOID != nullptr) {
                if (strcmp(IOID, filter) == 0) {
                    count++;
                }

                free(IOID);
            } else {
                // If IOID cannot be determined
                count++;
            }

            // If PCM device is found
            if ((count- 1) == aIndex) {
                NAME = snd_device_name_get_hint(*n, "NAME");
                DESC = snd_device_name_get_hint(*n, "DESC");

                temp.name = NAME;
                temp.description = DESC;

                temp.type = aType;

                temp.isValid = true;

                if (NAME != nullptr) {
                    free(NAME);
                }

                if (DESC != nullptr) {
                    free(DESC);
                }

                break;
            }
        }

        return temp;
    }

    void quit() {
        snd_device_name_free_hint(hints);
    }
}
#endif
