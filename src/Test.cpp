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
	
	assert(getDeviceCount(count, DeviceType::PLAYBACK) == Exit::SUCCESS);
	
	for (unsigned int i = 0; i < count; i++) {
		Device device;
		assert(openDevice(device, i, DeviceType::PLAYBACK) == Exit::SUCCESS);

		std::cout << "\nDevice name: " << device.name << '\n';
		
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
		
		std::cout << "Supported buffer formats: ";
		if ((device.supportedFormats & BufferFormat::U_INT_8) == BufferFormat::U_INT_8) {
			std::cout << "/ Unsigned 8-bit int ";
		}

		if ((device.supportedFormats & BufferFormat::S_INT_16) == BufferFormat::S_INT_16) {
			std::cout << "/ Signed 16-bit int ";
		}

		if ((device.supportedFormats & BufferFormat::S_INT_24) == BufferFormat::S_INT_24) {
			std::cout << "/ Signed 24-bit int ";
		}

		if ((device.supportedFormats & BufferFormat::S_INT_32) == BufferFormat::S_INT_32) {
			std::cout << "/ Signed 32-bit int ";
		}

		if ((device.supportedFormats & BufferFormat::FLOAT_32) == BufferFormat::FLOAT_32) {
			std::cout << "/ 32-bit float ";
		}

		if ((device.supportedFormats & BufferFormat::FLOAT_64) == BufferFormat::FLOAT_64) {
			std::cout << "/ 64-bit float ";
		}
		std::cout << "/\n";

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
