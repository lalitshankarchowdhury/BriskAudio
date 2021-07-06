#include "../include/BriskAudio.hpp"
#include <assert.h>
#include <iostream>

#define BLUE "\u001b[34m"
#define RESET "\u001b[0m"

using namespace BriskAudio;

void onDefaultDeviceChange(std::string aDeviceName, DeviceType aType)
{
    const char* type = (aType == DeviceType::PLAYBACK) ? "output" : "input";

    std::cout << BLUE << "Default " << type << " device changed to: " << aDeviceName + '\n' + RESET;
}

// TODO: Fix problem
void onDeviceAdd(std::string aDeviceName, DeviceType aType)
{
    const char* type = (aType == DeviceType::PLAYBACK) ? "Output" : "Input";

    std::cout << BLUE << type << " device added: " << aDeviceName + '\n' + RESET;
}

// TODO: Fix problem
void onDeviceRemove(std::string aDeviceName, DeviceType aType)
{
    const char* type = (aType == DeviceType::PLAYBACK) ? "Output" : "Input";

    std::cout << BLUE << type << " device removed: " << aDeviceName + '\n' + RESET;
}

int main()
{
    assert(init() == Exit::SUCCESS);

    // Register device event callbacks
    assert(registerDeviceEventCallbacks(onDefaultDeviceChange, onDeviceAdd, onDeviceRemove) == Exit::SUCCESS);

    std::getchar();

    assert(quit() == Exit::SUCCESS);
}
