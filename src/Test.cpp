#include "../include/BriskAudio.hpp"
#include "BriskAudioWindows.hpp"
#include <cassert>
#include <iostream>

using namespace BriskAudio;

int main()
{
    assert(init() == Exit::SUCCESS);

    DeviceEnumerator enumerator;

    enumerator.type = DeviceType::PLAYBACK;

    std::unique_ptr<Device> pDevice(enumerator.returnDefaultDevice());

    if (pDevice->isStreamFormatSupported(BufferFormat::S_INT_24, 2, 48000)) {
        std::cout << "Stream format supported\n";
    }
    else {
        std::cout << "Stream format not supported\n";
    }

    assert(quit() == Exit::SUCCESS);
}
