#include <iostream>
#include <gtest/gtest.h>

#include "Devices/NidaqDevice.h"
#include "Devices/Generic/Exceptions.h"

static double requiredPrecision(1e-10);

using namespace STIMWALKER_NAMESPACE;

// Start the tests

TEST(Nidaq, connect){
    auto lokomat = devices::NidaqDeviceMock(4, 1000);
    
    ASSERT_EQ(lokomat.getIsConnected(), false);
    
    lokomat.connect();
    ASSERT_EQ(lokomat.getIsConnected(), true);
    
    EXPECT_THROW(lokomat.connect(), devices::DeviceIsConnectedException);

    lokomat.startRecording();
    EXPECT_THROW(lokomat.disconnect(), devices::DeviceIsRecordingException);
    lokomat.stopRecording();

    lokomat.disconnect();
    ASSERT_EQ(lokomat.getIsConnected(), false);

    EXPECT_THROW(lokomat.disconnect(), devices::DeviceIsNotConnectedException);
}

TEST(Nidaq, recording){
    auto lokomat = devices::NidaqDeviceMock(4, 1000);
    ASSERT_EQ(lokomat.isRecording(), false);
    
    EXPECT_THROW(lokomat.startRecording(), devices::DeviceIsNotConnectedException);

    lokomat.connect();
    lokomat.startRecording();
    ASSERT_EQ(lokomat.isRecording(), true);

    EXPECT_THROW(lokomat.startRecording(), devices::DeviceIsRecordingException);
    
    lokomat.stopRecording();
    ASSERT_EQ(lokomat.isRecording(), false);

    EXPECT_THROW(lokomat.stopRecording(), devices::DeviceIsNotRecordingException);
}