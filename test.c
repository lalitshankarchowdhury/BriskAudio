#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

typedef enum {
    DEVICE_TYPE_INPUT,
    DEVICE_TYPE_OUTPUT
} DeviceType;

typedef struct {
    void* nativeDeviceHandle;
    int deviceID;
    bool isDefault;
    const char* name;
    DeviceType type;
    unsigned int channels;
    unsigned int numSamples;
    unsigned int sampleRate;
    unsigned int sampleFormat;
} Device;

typedef struct {
    int deviceID;
    const char* name;
} EnumeratedDevice;

#ifdef __linux__
#include <alsa/asoundlib.h>

int openDefaultDevice(Device* device) {
    int err = snd_pcm_open((snd_pcm_t**) &(device->nativeDeviceHandle),
                 "default", 
                 SND_PCM_STREAM_PLAYBACK, 
                 SND_PCM_NONBLOCK);

    if (err < 0) {
        return 1;
    }

    return 0;
}

int closeDefaultDevice(Device* device) {
    if (snd_pcm_close((snd_pcm_t*) device->nativeDeviceHandle) < 0) {
        return 1;
    }    

    return 0;
}
#endif

#ifdef _WIN32
#include <mmdeviceapi.h>

int openDefaultDevice(Device* device) {
    return 0;
}

int closeDefaultDevice(Device* device) {
    return 0;
}
#endif

int main() {
    printf("%lu\n", sizeof(Device));
    printf("%lu\n", sizeof(EnumeratedDevice));

    Device deviceHandle;

    assert(openDefaultDevice(&deviceHandle) == 0);
    assert(closeDefaultDevice(&deviceHandle) == 0);

    return EXIT_SUCCESS;
}
