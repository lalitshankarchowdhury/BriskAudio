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

	unsigned int count = 0;
	
	assert(getDeviceCount(count, DeviceType::CAPTURE) == Exit::SUCCESS);
	
	for (unsigned int i = 0; i < count; i++) {
		Device device;
		assert(openDevice(device, i, DeviceType::CAPTURE) == Exit::SUCCESS);

		std::cout << "Device name: " << device.name << '\n';
		
		std::cout << "Supported channels:";
		for (unsigned int numChannels : device.supportedChannels) {
			std::cout << ' ' << numChannels;
		}
		std::cout << '\n';

		std::cout << "Supported sample rates:";
		for (unsigned int sampleRate : device.sampleRates) {
			std::cout << ' ' << sampleRate;
		}
		std::cout << '\n';
		
		std::cout << "Buffer format mask: 0x" << std::hex << static_cast<unsigned int>(device.supportedFormats) << '\n';

		device.pOnVolumeChange = onVolumeChange;
		device.pOnMute = onMute;
		device.pOnDefaultDeviceChange = onDefaultDeviceChange;
		device.pOnDeviceAdd = onDeviceAdd;
		device.pOnDeviceRemove = onDeviceRemove;

		getchar();

		assert(closeDevice(device) == Exit::SUCCESS);
	}

    assert(quit() == Exit::SUCCESS);
}
