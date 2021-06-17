#include <iostream>

#include "BriskAudio.hpp"

int main() {
    BriskAudio::DeviceInfo info;

    std::cout << "Device count: " << info.getDeviceCount() << '\n';
}
