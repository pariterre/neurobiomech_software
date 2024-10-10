#ifndef __STIMWALKER_DEVICES_LOCOMAT_H__
#define __STIMWALKER_DEVICES_LOCOMAT_H__

#include "stimwalkerConfig.h"

#include "Devices/Concrete/NidaqDevice.h"

namespace STIMWALKER_NAMESPACE::devices {

class LokomatDevice : public NidaqDevice {
public:
  LokomatDevice() : NidaqDevice(25, std::chrono::milliseconds(1)) {}
};
} // namespace STIMWALKER_NAMESPACE::devices

#endif // __STIMWALKER_DEVICES_LOCOMAT_H__