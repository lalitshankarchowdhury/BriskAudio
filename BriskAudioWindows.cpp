#ifdef _WIN32
#include "BriskAudio.hpp"

#include <mmdeviceapi.h>

namespace BriskAudio {
	unsigned int DeviceEnumerator::getDeviceCount() {
		return 0;
	}

	DeviceInfo DeviceEnumerator::getDeviceInfo(unsigned int i) {
		return DeviceInfo();
	}
}
#endif
