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

void onDefaultDeviceChange(std::string aDeviceName)
{
    std::cout << "Default device changed to: " << aDeviceName << '\n';
}

void onDeviceAdd(std::string aDeviceName)
{
    std::cout << "Device added: " << aDeviceName << '\n';
}

void onDeviceRemove(std::string aDeviceName)
{
    std::cout << "Device removed: " << aDeviceName << '\n';
}

int main()
{
    assert(init() == Exit::SUCCESS);

    Device device;

    assert(openDefaultDevice(device, DeviceType::PLAYBACK) == Exit::SUCCESS);

    device.pOnVolumeChange = onVolumeChange;
    device.pOnMute = onMute;
    device.pOnDefaultDeviceChange = onDefaultDeviceChange;
    device.pOnDeviceAdd = onDeviceAdd;
    device.pOnDeviceRemove = onDeviceRemove;

    getchar();

    assert(closeDevice(device) == Exit::SUCCESS);

    assert(quit() == Exit::SUCCESS);
}
