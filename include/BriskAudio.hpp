#pragma once

#include <string>
#include <vector>

namespace BriskAudio {
enum class Exit {
    SUCCESS,
    FAILURE
};

enum class EndpointType {
    PLAYBACK,
    CAPTURE
};

enum class BufferFormat {
    U_INT_8,
    S_INT_16,
    S_INT_24,
    S_INT_32,
    FLOAT_32,
    FLOAT_64
};

struct StreamConfig {
    bool isValid;
    unsigned int numChannels;
    unsigned int sampleRate;
    BufferFormat format;

    StreamConfig()
    {
        isValid = false;
        numChannels = 0;
        sampleRate = 0;
        formats = (BufferFormat) 0;
    }
};

struct Stream {
    void* nativeHandle;

    Stream() {
        nativeHandle = nullptr;
    }

    unsigned int getBufferSize();
    Exit setBufferSize(unsigned int aSize);
    Exit start();
    Exit stop();

    private:
        unsigned int bufferSize_;
};

struct Endpoint {
    bool isValid;
    void* nativeHandle;
    std::string cardName;
    std::string description;
    EndpointType type;

    Endpoint()
    {
        isValid = false;
        nativeHandle = nullptr;
        cardName = "INVALID";
        description = "INVALID";
        type = (EndpointType) 0;
    }

    void releaseNativeHandle();
    StreamConfig getSupportedStreamConfigs();
    Exit openStream(Stream* aStream);
    Exit closeStream(Stream* aStream);
};

struct EndpointEnumerator {
    EndpointType type;

    unsigned int getEndpointCount();
    Endpoint getDefaultEndpoint();
    Endpoint getEndpoint(unsigned int aIndex);
};

Exit init();
Exit quit();
}
