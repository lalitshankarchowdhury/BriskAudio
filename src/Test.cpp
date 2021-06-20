#include <iostream>

#include "BriskAudio.hpp"

using namespace BriskAudio;

int main() {
    DeviceEnumerator deviceEnumerator;
    DeviceInfo deviceInfo;

    unsigned int playbackDeviceCount = deviceEnumerator.getDeviceCount(DeviceType::PLAYBACK);

    std::cout << "--------------------------------------------------------------------------------" << '\n';

    std::cout << "Playback device count: " << playbackDeviceCount << '\n';

    // Display device details if successfully probed
    for (unsigned int i = 0; i < playbackDeviceCount; i++) {
        deviceInfo = deviceEnumerator.getDeviceInfo(i, DeviceType::PLAYBACK);

        if (deviceInfo.isValid) {
            std::cout << "--------------------------------------------------------------------------------" << '\n';
            std::cout << "Name: " << deviceInfo.name << '\n';
            std::cout << "Description: " << deviceInfo.description << '\n';
        }
    }

    unsigned int captureDeviceCount = deviceEnumerator.getDeviceCount(DeviceType::CAPTURE);

    std::cout << "--------------------------------------------------------------------------------" << '\n';

    std::cout << "Capture device count: " << captureDeviceCount << '\n';

    // Display device details if successfully probed
    for (unsigned int i = 0; i < captureDeviceCount; i++) {
        deviceInfo = deviceEnumerator.getDeviceInfo(i, DeviceType::CAPTURE);

        if (deviceInfo.isValid) {
            std::cout << "--------------------------------------------------------------------------------" << '\n';
            std::cout << "Name: " << deviceInfo.name << '\n';
            std::cout << "Description: " << deviceInfo.description << '\n';
        }
    }

    std::cout << "--------------------------------------------------------------------------------" << '\n';

    std::getchar();
}
