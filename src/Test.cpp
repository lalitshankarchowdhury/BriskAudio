#include "../include/BriskAudio.hpp"
#include <cassert>
#include <iostream>

using namespace BriskAudio;

int main()
{
    assert(init() == Exit::SUCCESS);

    Device device;
    Stream stream;

    assert(openDefaultDevice(device, DeviceType::PLAYBACK) == Exit::SUCCESS);

    assert(device.openStream(stream, 2, 44100, BufferFormat::U_INT_8, 11.6f) == Exit::SUCCESS);

    std::cin.get();

    assert(closeDevice(device) == Exit::SUCCESS);

    assert(quit() == Exit::SUCCESS);
}
