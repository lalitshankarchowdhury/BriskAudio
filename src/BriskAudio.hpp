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

    struct EndpointInfo {
        bool isValid = false;
        std::string cardName;
        std::string description;
        EndpointType type;
    };

    struct EndpointInfoCollection {
        EndpointInfoCollection(EndpointType aType) {
            type_ = aType;
        }
        EndpointType getEndpointType() {
            return type_;
        }
        void setEndpointType(EndpointType aType) {
            type_ = aType;
        }
        unsigned int getEndpointCount();
        EndpointInfo getEndpointInfo(unsigned int aIndex);

    private:
        EndpointType type_;
    };

    Exit init();
    void quit();
}
