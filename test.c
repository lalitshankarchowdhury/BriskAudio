#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <mmdeviceapi.h>

#ifdef _linux_
typedef snd_pcm_t NativeDeviceHandle;
#endif

#ifdef _WIN32
typedef IMMDevice NativeDeviceHandle;
#endif

typedef enum {
    DEVICE_TYPE_INPUT,
    DEVICE_TYPE_OUTPUT
} DeviceType;

typedef struct {
    NativeDeviceHandle* handle;
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

#ifdef _linux_
int openDefaultDevice(Device* device) {
    if (snd_pcm_open(&(device->handle), "default", SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK) < 0) {
        return 1;
    }

    return 0;
}

int closeDefaultDevice(Device* device) {
    if (snd_pcm_close(&(device->handle)) < 0) {
        return 1;
    }    

    return 0;
}
#endif

#ifdef _WIN32
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