#include <cassert>
#include <iostream>
#include "../include/BriskAudio.hpp"

using namespace BriskAudio;

void onVolumeChange(float aVolume)
{
    std::cout << "Volume changed: " << aVolume * 100.0f << "%\n";
}

void onMute()
{
    std::cout << "Device muted\n";
}

int main()
{
    assert(init() == Exit::SUCCESS);

    Device device;

    assert(openDefaultDevice(device, DeviceType::PLAYBACK) == Exit::SUCCESS);

    device.pOnVolumeChange = onVolumeChange;
    device.pOnMute = onMute;

    getchar();

    assert(closeDevice(device) == Exit::SUCCESS);

    assert(quit() == Exit::SUCCESS);
}