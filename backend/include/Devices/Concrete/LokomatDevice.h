#ifndef __NEUROBIO_DEVICES_LOCOMAT_H__
#define __NEUROBIO_DEVICES_LOCOMAT_H__

#include "neurobioConfig.h"

#include "Devices/Concrete/NidaqDevice.h"

namespace NEUROBIO_NAMESPACE::devices {

class LokomatDevice : public NidaqDevice {
public:
  LokomatDevice() : NidaqDevice(25, std::chrono::milliseconds(1)) {}
};
} // namespace NEUROBIO_NAMESPACE::devices

#endif // __NEUROBIO_DEVICES_LOCOMAT_H__