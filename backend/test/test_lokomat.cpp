#include <iostream>
#include <gtest/gtest.h>

#include "Devices/Lokomat.h"
#include "Devices/Generic/Exceptions.h"

static double requiredPrecision(1e-10);

using namespace STIMWALKER_NAMESPACE;

// Start the tests

TEST(Lokomat, factory){
    bool isMock = true;
    auto lokomat = devices::makeLokomat(isMock);
    
    ASSERT_EQ(lokomat.getNbChannels(), 25);
    ASSERT_EQ(lokomat.getFrameRate(), 1000);
}
