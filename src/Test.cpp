#include <iostream>

#include "BriskAudio.hpp"

using namespace BriskAudio;

int main() {
    DeviceInfoCollection collection;
    DeviceInfo info;

    unsigned int playbackDeviceCount = collection.getDeviceCount(DeviceType::PLAYBACK);

    std::cout << "\nPlayback device count: " << playbackDeviceCount << "\n\n";

    // Display playback device details if successfully probed
    for (unsigned int i = 0; i < playbackDeviceCount; i++) {
        info = collection.getDeviceInfo(i, DeviceType::PLAYBACK);

        if (info.isValid) {
            std::cout << "Name: " << info.name << '\n';
            std::cout << "Description: " << info.description << "\n\n";
        }
    }

    unsigned int captureDeviceCount = collection.getDeviceCount(DeviceType::CAPTURE);

    std::cout << "Capture device count: " << captureDeviceCount << "\n\n";

    // Display capture device details if successfully probed
    for (unsigned int i = 0; i < captureDeviceCount; i++) {
        info = collection.getDeviceInfo(i, DeviceType::CAPTURE);

        if (info.isValid) {
            std::cout << "Name: " << info.name << '\n';
            std::cout << "Description: " << info.description << "\n\n";
        }
    }

    std::getchar();
}
