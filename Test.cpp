#include <cassert>
#include <iostream>
#include "BriskAudio.hpp"

using namespace BriskAudio;

int main()
{
    assert(init() == Exit::SUCCESS);

    Device device;

    assert(openDefaultDevice(device, DeviceType::PLAYBACK) == Exit::SUCCESS);

    assert(closeDevice(device) == Exit::SUCCESS);

    getchar();

    assert(quit() == Exit::SUCCESS);
}