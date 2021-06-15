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
    const char* ID;
    bool isDefault;
    const char* name;
    DeviceType type;
    unsigned int channels;
    unsigned int numSamples;
    unsigned int sampleRate;
    unsigned int sampleFormat;
} Device;

typedef struct {
    const char* ID;
    const char* name;
} EnumeratedDevice;

#ifdef __linux__
#include <alsa/asoundlib.h>

int openDefaultDevice(Device* deviceHandle) {
    int err = snd_pcm_open(deviceHandle == NULL || (snd_pcm_t**) &(deviceHandle->nativeDeviceHandle),
                 "default", 
                 SND_PCM_STREAM_PLAYBACK, 
                 SND_PCM_NONBLOCK);

    if (err < 0) {
        return 1;
    }

    return 0;
}

EnumeratedDevice* enumerateDevices() {
    char **hints;
    int enumeratedDeviceArraySize = 4;
    EnumerateDevice* enumeratedDevices = (EnumerateDevice*) malloc(enumeratedDeviceArraySize * sizeof(EnumerateDevice));

    if (enumeratedDevices == NULL) {
        free(enumeratedDevices);

        return NULL;
    }

    if (snd_device_name_hint(-1, "pcm", (void***) &hints) < 0) {
        free(enumeratedDevices);
        
        return NULL;
    }

    int cnt = 0;

    while (*hints != NULL) {
        if (cnt == enumeratedDeviceArraySize) {
            enumeratedDeviceArraySize += 1;

            EnumeratedDevice* temp = realloc(enumeratedDevices, enumeratedDeviceArraySize);

            if (temp == NULL) {
                free(enumeratedDevices);
        
                return NULL;
            }
        }

        enumeratedDevices[cnt].ID = snd_device_name_get_hint(*hints, "IOID");
        enumeratedDevices[cnt].name = snd_device_name_get_hint(*hints, "NAME");

        cnt++;
        hints++;
    }

    return enumeratedDevices;
}

int closeDefaultDevice(Device* deviceHandle) {
    if (snd_pcm_close((snd_pcm_t*) deviceHandle->nativeDeviceHandle) < 0) {
        return 1;
    }    

    return 0;
}
#endif

#ifdef _WIN32
#include <initguid.h>
#include <Audioclient.h>
#include <mmdeviceapi.h>

DEFINE_GUID(CLSID_MMDeviceEnumerator, 0xBCDE0395, 0xE52F, 0x467C, 0x8E, 0x3D, 0xC4, 0x57, 0x92, 0x91, 0x69, 0x2E);
DEFINE_GUID(IID_IMMDeviceEnumerator,  0xA95664D2, 0x9614, 0x4F35, 0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6);
DEFINE_GUID(IID_IAudioClient, 0x1CB9AD4C, 0xDBFA, 0x4c32, 0xB1, 0x78, 0xC2, 0xF5, 0x68, 0xA7, 0x03, 0xB2);

#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->lpVtbl->Release(punk); (punk) = NULL; }

int openDefaultDevice(Device* deviceHandle) {
    int err = 0;

    CoInitialize(NULL);

    IMMDeviceEnumerator* enumerator = NULL;

    if (FAILED(CoCreateInstance(&CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, &IID_IMMDeviceEnumerator, (void**) &enumerator))) {
        err = 1;

        goto Exit;
    }

    IMMDevice* device = NULL;

    if (FAILED(enumerator->lpVtbl->GetDefaultAudioEndpoint(enumerator, eRender, eConsole, &device))) {
        err = 1;

        goto Exit;
    }

    deviceHandle->nativeDeviceHandle = device;

Exit:
    SAFE_RELEASE(enumerator);

    return err;
}

EnumeratedDevice* enumerateDevices() {
    return NULL;
}

int closeDefaultDevice(Device* deviceHandle) {
    if (deviceHandle == NULL) {
        return 1;
    }

    CoUninitialize();

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
