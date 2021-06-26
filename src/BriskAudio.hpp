#pragma once

#include <string>

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

        Endpoint() {
            isValid = false;
            nativeHandle = nullptr;
        }

        void releaseNativeHandle();
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
