#include <iostream>
#include "BriskAudio.hpp"

#define GREEN "\u001b[32m"
#define RESET "\u001b[0m"

using namespace BriskAudio;

int main() {
    if (init() == Exit::FAILURE) {
        return EXIT_FAILURE;
    }

    DeviceInfoCollection collection;
    DeviceInfo info;

    unsigned int playbackDeviceCount = collection.getDeviceCount(DeviceType::PLAYBACK);

    std::cout << "--------------------------------------------------------------------------------\n";
    std::cout << "\nPlayback device count: " << playbackDeviceCount << "\n\n";
    std::cout << "--------------------------------------------------------------------------------\n";

    // Display playback device details if successfully probed
    for (unsigned int i = 0; i < playbackDeviceCount; i++) {
        info = collection.getDeviceInfo(i, DeviceType::PLAYBACK);

        if (info.isValid) {
            std::cout << GREEN << info.name << RESET << '\n';
            std::cout << info.description << '\n';
            std::cout << "--------------------------------------------------------------------------------\n";
        }
    }

    unsigned int captureDeviceCount = collection.getDeviceCount(DeviceType::CAPTURE);

    std::cout << "\nCapture device count: " << captureDeviceCount << "\n\n";
    std::cout << "--------------------------------------------------------------------------------\n";

    // Display capture device details if successfully probed
    for (unsigned int i = 0; i < captureDeviceCount; i++) {
        info = collection.getDeviceInfo(i, DeviceType::CAPTURE);

        if (info.isValid) {
            std::cout << GREEN << info.name << RESET << '\n';
            std::cout << info.description << '\n';
            std::cout << "--------------------------------------------------------------------------------\n";
        }
    }

    std::getchar();

    quit();

    return EXIT_SUCCESS;
}
