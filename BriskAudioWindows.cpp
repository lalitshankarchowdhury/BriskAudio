#ifdef _WIN32
#include <mmdeviceapi.h>

namespace BriskAudio {
	unsigned int DeviceEnumerator::getDeviceCount() {
		return 0;
	}
}
#endif
