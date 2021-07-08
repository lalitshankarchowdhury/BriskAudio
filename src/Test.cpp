#include "../include/BriskAudio.hpp"
#include <assert.h>
#include <iostream>

using namespace BriskAudio;

void onDefaultDeviceChange(std::string aDeviceName, DeviceType aType)
{
    const char* type = (aType == DeviceType::PLAYBACK) ? "output" : "input";

    std::cout << "Default " << type << " device changed to: " << aDeviceName + '\n';
}

void onDeviceAdd(std::string aDeviceName, DeviceType aType)
{
    const char* type = (aType == DeviceType::PLAYBACK) ? "Output" : "Input";

    std::cout << type << " device added: " << aDeviceName + '\n';
}

void onDeviceRemove(std::string aDeviceName, DeviceType aType)
{
    const char* type = (aType == DeviceType::PLAYBACK) ? "Output" : "Input";

    std::cout << type << " device removed: " << aDeviceName + '\n';
}

int main()
{
    assert(init() == Exit::SUCCESS);

    // Register device callbacks (optional)
    pOnDefaultDeviceChange = onDefaultDeviceChange;
    pOnDeviceAdd = onDeviceAdd;
    pOnDeviceRemove = onDeviceRemove;

    std::getchar();

    assert(quit() == Exit::SUCCESS);
}
