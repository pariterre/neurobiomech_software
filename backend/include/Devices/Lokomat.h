#ifndef __STIMWALKER_DEVICES_LOCOMAT_H__
#define __STIMWALKER_DEVICES_LOCOMAT_H__

#include "Devices/NidaqDevice.h"
#include "stimwalkerConfig.h"

namespace STIMWALKER_NAMESPACE::devices {
std::unique_ptr<NidaqDevice> makeLokomatDevice(bool isMock) {
  int nbChannels = 25;
  std::chrono::milliseconds acquisitionTimer = std::chrono::milliseconds(1);

  return isMock ? throw std::runtime_error("Mock not implemented yet")
                : std::make_unique<NidaqDevice>(nbChannels, acquisitionTimer);
}
} // namespace STIMWALKER_NAMESPACE::devices

#endif // __STIMWALKER_DEVICES_LOCOMAT_H__