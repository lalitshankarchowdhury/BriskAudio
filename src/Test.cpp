#include "../include/BriskAudio.hpp"
#include "BriskAudioWindows.hpp"
#include <cassert>
#include <iostream>

using namespace BriskAudio;

int main()
{
    assert(init() == Exit::SUCCESS);

    DeviceEnumerator enumerator;
    Device* pDevice = nullptr;

    enumerator.type = DeviceType::PLAYBACK;

    std::cout << "--------------------------------------------------------------------\n";

    for (unsigned int i = 0; i < enumerator.returnDeviceCount(); i++) {
        pDevice = enumerator.returnDevice(i);

        assert(pDevice != nullptr);

        std::cout << "Name: " << pDevice->name << '\n';
        std::cout << "Default sample rate: " << pDevice->defaultSampleRate << "Hz\n";
        std::cout << "Supported sample rates: ";
        for (unsigned int sampleRate : pDevice->supportedSampleRates) {
            std::cout << sampleRate << "Hz ";
        }
        std::cout << '\n';
        std::cout << "Supported channels: ";
        for (unsigned int numChannels : pDevice->supportedNumChannels) {
            std::cout << numChannels << " ";
        }
        std::cout << '\n';
        std::cout << "--------------------------------------------------------------------\n";

        delete pDevice;
    }

    assert(quit() == Exit::SUCCESS);
}
