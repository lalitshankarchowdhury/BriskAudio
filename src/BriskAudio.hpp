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
        EndpointEnumerator(EndpointType aType) {
            type_ = aType;
        }
        EndpointType getEndpointType() {
            return type_;
        }
        void setEndpointType(EndpointType aType) {
            type_ = aType;
        }
        unsigned int getEndpointCount();
        Endpoint getDefaultEndpoint();
        Endpoint getEndpoint(unsigned int aIndex);

    private:
        EndpointType type_;
    };
    
    Exit init();
    Exit quit();
}
