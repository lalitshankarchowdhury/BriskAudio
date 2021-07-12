#include "../include/BriskAudio.hpp"
#include "BriskAudioWindows.hpp"
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

    DeviceEventNotifier notifier1;
    DeviceEventNotifier notifier2;

    assert(notifier1.registerEventCallbacks(onDefaultDeviceChange, onDeviceAdd, onDeviceRemove) == Exit::SUCCESS);
    assert(notifier2.registerEventCallbacks(onDefaultDeviceChange, onDeviceAdd, onDeviceRemove) == Exit::SUCCESS);

    getchar();

    assert(notifier1.unregisterEventCallbacks() == Exit::SUCCESS);
    assert(notifier2.unregisterEventCallbacks() == Exit::SUCCESS);

    assert(quit() == Exit::SUCCESS);
}
