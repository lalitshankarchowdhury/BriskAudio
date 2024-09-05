#include "../include/BriskAudio.hpp"
#include <cassert>
#include <iostream>

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define GRAY "\033[90m"
#define DEFAULT "\033[0m"

void onPlaybackDeviceMuted(std::string deviceName) {
	std::cerr << YELLOW "MUTED PLAYBACK DEVICE: " << deviceName << DEFAULT << std::endl;
}

void onCaptureDeviceMuted(std::string deviceName) {
	std::cerr << YELLOW "MUTED CAPTURE DEVICE: " << deviceName << DEFAULT << std::endl;
}

void onDefaultPlaybackDeviceChanged(std::string deviceName) {
	std::cerr << BLUE "DEFAULT PLAYBACK DEVICE: " << deviceName << DEFAULT << std::endl;
}

void onDefaultCaptureDeviceChanged(std::string deviceName) {
	std::cerr << BLUE "DEFAULT CAPTURE DEVICE: " << deviceName << DEFAULT << std::endl;
}

void onAnyDeviceAdded(std::string deviceName) {
	std::cerr << GREEN "ADDED DEVICE: " << deviceName << DEFAULT << std::endl;
}

void onAnyDeviceRemoved(std::string deviceName) {
	std::cerr << RED "REMOVED DEVICE: " << deviceName << DEFAULT << std::endl;
}

void onAnyDeviceStateChange(std::string deviceName, std::string state) {
	std::cerr << GRAY << state << " DEVICE: " << deviceName << DEFAULT << std::endl;
}

int main()
{
	if (BriskAudio::init() == BriskAudio::Exit::FAILURE) {
		std::cerr << "Failed to initialize BriskAudio";
		return EXIT_FAILURE;
	}

	BriskAudio::Device playbackDevice, captureDevice;

	if (openDefaultDevice(playbackDevice, BriskAudio::DeviceType::PLAYBACK) == BriskAudio::Exit::FAILURE) {
		std::cerr << "Failed to open default playback device";
		BriskAudio::quit();
		return EXIT_FAILURE;
	}

	if (openDefaultDevice(captureDevice, BriskAudio::DeviceType::CAPTURE) == BriskAudio::Exit::FAILURE) {
		std::cerr << "Failed to open default capture device";
		BriskAudio::quit();
		return EXIT_FAILURE;
	}

	playbackDevice.pOnMute = onPlaybackDeviceMuted;
	playbackDevice.pOnDefaultDeviceChange = onDefaultPlaybackDeviceChanged;
	playbackDevice.pOnAnyDeviceAdd = onAnyDeviceAdded;
	playbackDevice.pOnAnyDeviceRemove = onAnyDeviceRemoved;
	playbackDevice.pOnAnyDeviceStateChange = onAnyDeviceStateChange;
	captureDevice.pOnMute = onCaptureDeviceMuted;
	captureDevice.pOnDefaultDeviceChange = onDefaultCaptureDeviceChanged;

	std::cout << "Press any key to stop monitoring..." << std::endl;
	std::cin.get();

	if (closeDevice(playbackDevice) == BriskAudio::Exit::FAILURE) {
		std::cerr << "Failed to close default playback device";
		BriskAudio::quit();
		return EXIT_FAILURE;
	}

	if (closeDevice(captureDevice) == BriskAudio::Exit::FAILURE) {
		std::cerr << "Failed to close default capture device";
		BriskAudio::quit();
		return EXIT_FAILURE;
	}

	BriskAudio::quit();
}
