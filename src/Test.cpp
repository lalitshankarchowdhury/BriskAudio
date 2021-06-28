#include <iostream>
#include "../include/BriskAudio.hpp"

#define GREEN "\u001b[32m"
#define RESET "\u001b[0m"

using namespace BriskAudio;

int main()
{
    if (init() == Exit::FAILURE) {
        return EXIT_FAILURE;
    }

    EndpointEnumerator enumerator;
    Endpoint endpoint;
    StreamConfig config;

    enumerator.type = EndpointType::PLAYBACK;

    endpoint = enumerator.getDefaultEndpoint();

    if (endpoint.isValid) {
        config = endpoint.getSupportedStreamConfigs();

        if (config.isValid) {
            std::cout << config.numChannels << '\n';
            std::cout << config.sampleRate << '\n';
        }

        endpoint.releaseNativeHandle();
    }

    if (quit() == Exit::FAILURE) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
