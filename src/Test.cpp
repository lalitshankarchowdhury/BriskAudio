#include <cassert>
#include <iostream>
#include "../include/BriskAudio.hpp"

using namespace BriskAudio;

int main()
{
    assert(init() == Exit::SUCCESS);

    Device device;
    float volume = 0.0f;

    assert(openDefaultDevice(device, DeviceType::PLAYBACK) == Exit::SUCCESS);
    assert(device.getVolume(volume) == Exit::SUCCESS);

    std::cout << device.name << '\n';
    std::cout << "Current volume: " << volume * 100.0f << "%\n";

    assert(device.setVolume(1.0f) == Exit::SUCCESS);

    std::cout << "Current volume: " << volume * 100.0f << "%\n";

    assert(closeDevice(device) == Exit::SUCCESS);

    getchar();

    assert(quit() == Exit::SUCCESS);
}
