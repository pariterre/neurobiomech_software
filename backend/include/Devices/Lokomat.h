#ifndef __STIMWALKER_DEVICES_LOCOMAT_H__
#define __STIMWALKER_DEVICES_LOCOMAT_H__

#include "Devices/NidaqDevice.h"
#include "stimwalkerConfig.h"

namespace STIMWALKER_NAMESPACE::devices {
std::unique_ptr<NidaqDevice> makeLokomatDevice(bool isMock) {
  int nbChannels = 25;
  int frameRate = 1000;

  return isMock ? throw std::runtime_error("Mock not implemented yet")
                : std::make_unique<NidaqDevice>(nbChannels, frameRate);
}
} // namespace STIMWALKER_NAMESPACE::devices

#endif // __STIMWALKER_DEVICES_LOCOMAT_H__