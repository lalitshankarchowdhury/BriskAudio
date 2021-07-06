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
        cardName = "INVALID";
        description = "INVALID";
        type = (EndpointType)0;
    }

    void releaseNativeHandle();
};

struct EndpointEnumerator {
    EndpointType type;

    EndpointEnumerator()
    {
        type = (EndpointType)0;
    }

    unsigned int getEndpointCount();
    Endpoint getDefaultEndpoint();
    Endpoint getEndpoint(unsigned int aIndex);
};

Exit init();
Exit quit();
}
