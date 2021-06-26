#include <iostream>
#include "../include/BriskAudio.hpp"

#define GREEN "\u001b[32m"
#define RESET "\u001b[0m"

using namespace BriskAudio;

int main() {
    if (init() == Exit::FAILURE) {
        return EXIT_FAILURE;
    }

    // List available endpoints

    EndpointEnumerator enumerator(EndpointType::PLAYBACK);
    Endpoint endpoint;

    std::cout << GREEN <<  "--------------------------------------------------------------------------------\n" << RESET;

    for (unsigned int i = 0; i < enumerator.getEndpointCount(); i++) {
        endpoint = enumerator.getEndpoint(i);

        if (endpoint.isValid) {
            std::cout << endpoint.cardName << '\n';
            std::cout << endpoint.description << '\n';
            std::cout << GREEN <<  "--------------------------------------------------------------------------------\n" << RESET;

            endpoint.releaseNativeHandle();
        }
    }

    enumerator.setEndpointType(EndpointType::CAPTURE);

    for (unsigned int i = 0; i < enumerator.getEndpointCount(); i++) {
        endpoint = enumerator.getEndpoint(i);

        if (endpoint.isValid) {
            std::cout << endpoint.cardName << '\n';
            std::cout << endpoint.description << '\n';
            std::cout << GREEN <<  "--------------------------------------------------------------------------------\n" << RESET;

            endpoint.releaseNativeHandle();
        }
    }

    std::getchar();

    if (quit() == Exit::FAILURE) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
