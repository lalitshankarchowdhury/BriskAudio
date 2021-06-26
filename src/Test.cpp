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

    EndpointInfoCollection collection(EndpointType::PLAYBACK);
    EndpointInfo info;

    std::cout << GREEN <<  "--------------------------------------------------------------------------------\n" << RESET;

    for (unsigned int i = 0; i < collection.getEndpointCount(); i++) {
        info = collection.getEndpointInfo(i);

        if (info.isValid) {
            std::cout << info.cardName << '\n';
            std::cout << info.description << '\n';
            std::cout << GREEN <<  "--------------------------------------------------------------------------------\n" << RESET;
        }
    }

    collection.setEndpointType(EndpointType::CAPTURE);

    for (unsigned int i = 0; i < collection.getEndpointCount(); i++) {
        info = collection.getEndpointInfo(i);

        if (info.isValid) {
            std::cout << info.cardName << '\n';
            std::cout << info.description << '\n';
            std::cout << GREEN <<  "--------------------------------------------------------------------------------\n" << RESET;
        }
    }

    std::getchar();

    quit();

    return EXIT_SUCCESS;
}
