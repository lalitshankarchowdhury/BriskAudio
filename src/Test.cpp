#include <iostream>

#include "BriskAudio.hpp"

using namespace BriskAudio;

int main() {
    DeviceEnumerator deviceEnumerator;
    DeviceInfo deviceInfo;

    unsigned int playbackDeviceCount = deviceEnumerator.getDeviceCount(DeviceType::PLAYBACK);

    std::cout << "\nPlayback device count: " << playbackDeviceCount << "\n\n";

    // Display playback device details if successfully probed
    for (unsigned int i = 0; i < playbackDeviceCount; i++) {
        deviceInfo = deviceEnumerator.getDeviceInfo(i, DeviceType::PLAYBACK);

        if (deviceInfo.isValid) {
            std::cout << "Name: " << deviceInfo.name << '\n';
            std::cout << "Description: " << deviceInfo.description << "\n\n";
        }
    }

    unsigned int captureDeviceCount = deviceEnumerator.getDeviceCount(DeviceType::CAPTURE);

    std::cout << "Capture device count: " << captureDeviceCount << "\n\n";

    // Display capture device details if successfully probed
    for (unsigned int i = 0; i < captureDeviceCount; i++) {
        deviceInfo = deviceEnumerator.getDeviceInfo(i, DeviceType::CAPTURE);

        if (deviceInfo.isValid) {
            std::cout << "Name: " << deviceInfo.name << '\n';
            std::cout << "Description: " << deviceInfo.description << "\n\n";
        }
    }

    std::getchar();
}
