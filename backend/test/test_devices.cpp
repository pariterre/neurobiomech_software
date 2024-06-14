#include <iostream>
#include <gtest/gtest.h>

#include "Devices/NidaqDevice.h"
#include "Devices/Generic/Exceptions.h"

static double requiredPrecision(1e-10);

using namespace STIMWALKER_NAMESPACE;

// Start the tests

TEST(Nidaq, channels){
    auto nidaq = devices::NidaqDeviceMock(4, 1000);
    
    ASSERT_EQ(nidaq.getNbChannels(), 4);
    ASSERT_EQ(nidaq.getFrameRate(), 1000);

}

TEST(Nidaq, connect){
    auto nidaq = devices::NidaqDeviceMock(4, 1000);
    
    ASSERT_EQ(nidaq.getIsConnected(), false);
    
    nidaq.connect();
    ASSERT_EQ(nidaq.getIsConnected(), true);
    
    EXPECT_THROW(nidaq.connect(), devices::DeviceIsConnectedException);

    nidaq.startRecording();
    EXPECT_THROW(nidaq.disconnect(), devices::DeviceIsRecordingException);
    nidaq.stopRecording();

    nidaq.disconnect();
    ASSERT_EQ(nidaq.getIsConnected(), false);

    EXPECT_THROW(nidaq.disconnect(), devices::DeviceIsNotConnectedException);
}

TEST(Nidaq, recording){
    auto nidaq = devices::NidaqDeviceMock(4, 1000);
    ASSERT_EQ(nidaq.isRecording(), false);
    
    EXPECT_THROW(nidaq.startRecording(), devices::DeviceIsNotConnectedException);

    nidaq.connect();
    nidaq.startRecording();
    ASSERT_EQ(nidaq.isRecording(), true);

    EXPECT_THROW(nidaq.startRecording(), devices::DeviceIsRecordingException);
    
    nidaq.stopRecording();
    ASSERT_EQ(nidaq.isRecording(), false);

    EXPECT_THROW(nidaq.stopRecording(), devices::DeviceIsNotRecordingException);
}