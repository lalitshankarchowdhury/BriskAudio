#include <iostream>

#include "BriskAudio.hpp"

int main() {
    BriskAudio::DeviceEnumerator deviceEnumerator;
    BriskAudio::DeviceInfo deviceInfo;

    unsigned int deviceCount = deviceEnumerator.getDeviceCount();

    std::cout << "Device count: " << deviceCount << '\n';

    // Display device details if successfully probed
    for (unsigned int i = 0; i < deviceCount; i++) {
        deviceInfo = deviceEnumerator.getDeviceInfo(i);

        if (deviceInfo.isValid) {
            std::cout << "--------------------------------------------------------------------------------" << '\n';
            std::cout << "Name: " << deviceInfo.name << '\n';
            std::cout << "Description: " << deviceInfo.description << '\n';
        }
    }

    std::cout << "--------------------------------------------------------------------------------" << '\n';

    std::getchar();
}
