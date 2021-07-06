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

    for (unsigned int i = 0; i < enumerator.getEndpointCount(); i++) {
        endpoint = enumerator.getEndpoint(i);

        if (endpoint.isValid) {
            std::cout << GREEN << endpoint.cardName + " - " + endpoint.description << RESET << '\n';
        }

        endpoint.releaseNativeHandle();
    }

    enumerator.type = EndpointType::CAPTURE;

    for (unsigned int i = 0; i < enumerator.getEndpointCount(); i++) {
        endpoint = enumerator.getEndpoint(i);

        if (endpoint.isValid) {
            std::cout << GREEN << endpoint.cardName + " - " + endpoint.description << RESET << '\n';
        }

        endpoint.releaseNativeHandle();
    }

    std::getchar();

    if (quit() == Exit::FAILURE) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
