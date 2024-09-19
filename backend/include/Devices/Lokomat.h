#ifndef __STIMWALKER_DEVICES_LOCOMAT_H__
#define __STIMWALKER_DEVICES_LOCOMAT_H__

#include "stimwalkerConfig.h"
#include "Devices/NidaqDevice.h"

namespace STIMWALKER_NAMESPACE::devices
{
    std::unique_ptr<NidaqDevice> makeLokomatDevice(bool isMock)
    {
        int nbChannels = 25;
        int frameRate = 1000;

        return isMock
                   ? std::make_unique<NidaqDeviceMock>(nbChannels, frameRate)
                   : std::make_unique<NidaqDevice>(nbChannels, frameRate);
    }
}

#endif // __STIMWALKER_DEVICES_LOCOMAT_H__