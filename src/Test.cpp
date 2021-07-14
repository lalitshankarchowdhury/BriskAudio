#include <cassert>
#include <iostream>
#include "../include/BriskAudio.hpp"

using namespace BriskAudio;

int main()
{
    assert(init() == Exit::SUCCESS);

    Device device;

    assert(openDevice(device, 0, DeviceType::PLAYBACK) == Exit::SUCCESS);

    std::cout << device.name << '\n';

    assert(closeDevice(device) == Exit::SUCCESS);

    assert(openDefaultDevice(device, DeviceType::PLAYBACK) == Exit::SUCCESS);

    std::cout << device.name << '\n';

    assert(closeDevice(device) == Exit::SUCCESS);

    assert(openDevice(device, "Speakers (Realtek(R) Audio)") == Exit::SUCCESS);

    std::cout << device.name << '\n';

    assert(closeDevice(device) == Exit::SUCCESS);

    getchar();

    assert(quit() == Exit::SUCCESS);
}
