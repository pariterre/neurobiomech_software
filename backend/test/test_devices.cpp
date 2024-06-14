#include <iostream>
#include <gtest/gtest.h>

#include "Devices/NidaqDevice.h"

static double requiredPrecision(1e-10);

using namespace STIMWALKER_NAMESPACE;

// Start the tests

TEST(Tata, tata){
    auto lokomat = stimwalker::devices::NidaqDevice();
    lokomat.connect();
}