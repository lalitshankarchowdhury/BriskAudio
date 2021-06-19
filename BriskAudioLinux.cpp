#include <string>
#ifdef __linux__
#include "BriskAudio.hpp"

#include <alsa/asoundlib.h>

namespace BriskAudio {
	unsigned int DeviceEnumerator::getDeviceCount() {
		return 10;
	}
	DeviceInfo DeviceEnumerator::getDeviceInfo(unsigned int i) {
		DeviceInfo temp;

		return temp;
	}
}
#endif
