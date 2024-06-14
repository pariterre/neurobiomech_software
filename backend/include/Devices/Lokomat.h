#ifndef LOCOMAT_H
#define LOCOMAT_H

#include "stimwalkerConfig.h"
#include "Devices/NidaqDevice.h"

namespace STIMWALKER_NAMESPACE{ namespace devices {

    NidaqDevice makeLokomat(bool isMock){
        int nbChannels = 25;
        int frameRate = 1000;

        return isMock ? NidaqDeviceMock(nbChannels, frameRate) : NidaqDevice(nbChannels, frameRate);
    }

}}

#endif // LOCOMAT_H