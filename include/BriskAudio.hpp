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
        cardName = nullptr;
        description = nullptr;
        type = EndpointType::PLAYBACK;
    }

    void releaseNativeHandle();
};

struct EndpointEnumerator {
    EndpointType type;

    EndpointEnumerator()
    {
        type = EndpointType::PLAYBACK;
    }

    unsigned int getEndpointCount();
    Endpoint getDefaultEndpoint();
    Endpoint getEndpoint(unsigned int aIndex);
};

Exit init();
Exit quit();
}
