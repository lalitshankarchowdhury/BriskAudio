#include <iostream>

#include "BriskAudio.hpp"

int main() {
    BriskAudio::DeviceEnumerator deviceEnumerator;
    BriskAudio::DeviceInfo deviceInfo;

    for (int i = 0; i < deviceEnumerator.getDeviceCount(); i++) {
        deviceInfo = deviceEnumerator.getDeviceInfo(i);

        if (deviceInfo.isValid) {
            std::cout << "Name: " << deviceInfo.name << '\n';
            std::cout << "Description: " << deviceInfo.description << '\n';
            unsigned int i  = deviceInfo.supportedSampleRates[0];
        }
    }
}
