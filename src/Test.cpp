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

    // List available endpoints

    EndpointEnumerator enumerator;
    Endpoint endpoint;

    enumerator.type = EndpointType::PLAYBACK;

    std::cout << GREEN << "--------------------------------------------------------------------------------\n" << RESET;

    for (unsigned int i = 0; i < enumerator.getEndpointCount(); i++) {
        endpoint = enumerator.getEndpoint(i);

        if (endpoint.isValid) {
            std::cout << endpoint.cardName << '\n';
            std::cout << endpoint.description << '\n';
            std::cout << GREEN << "--------------------------------------------------------------------------------\n" << RESET;

            // Release OS specific endpoint handle
            endpoint.releaseNativeHandle();
        }
    }

    enumerator.type = EndpointType::CAPTURE;

    for (unsigned int i = 0; i < enumerator.getEndpointCount(); i++) {
        endpoint = enumerator.getEndpoint(i);

        if (endpoint.isValid) {
            std::cout << endpoint.cardName << '\n';
            std::cout << endpoint.description << '\n';
            std::cout << GREEN << "--------------------------------------------------------------------------------\n" << RESET;

            // Release OS specific endpoint handle
            endpoint.releaseNativeHandle();
        }
    }

    std::getchar();

    if (quit() == Exit::FAILURE) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
