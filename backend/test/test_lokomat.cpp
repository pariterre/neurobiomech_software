#include <gtest/gtest.h>
#include <iostream>

#include "Devices/Generic/Exceptions.h"
#include "Devices/Lokomat.h"

static double requiredPrecision(1e-10);

using namespace STIMWALKER_NAMESPACE;

// Start the tests

TEST(Lokomat, factory) {
  bool isMock = true;
  auto lokomat = devices::makeLokomatDevice(isMock);

  ASSERT_EQ(lokomat->getNbChannels(), 25);
  ASSERT_EQ(lokomat->getFrameRate(), 1000);
}
