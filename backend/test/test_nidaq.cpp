// #include <gtest/gtest.h>
// #include <iostream>

// #include "Devices/Generic/Exceptions.h"
// #include "Devices/NidaqDevice.h"

// static double requiredPrecision(1e-10);

// using namespace STIMWALKER_NAMESPACE;

// // Start the tests

// TEST(Nidaq, channels) {
//   // TODO Change these back to mocks
//   auto nidaq = devices::NidaqDevice(4, 1000);

//   ASSERT_EQ(nidaq.getDataChannelCount(), 4);
//   ASSERT_EQ(nidaq.getFrameRate(), 1000);
// }

// TEST(Nidaq, connect) {
//   auto nidaq = devices::NidaqDevice(4, 1000);

//   ASSERT_EQ(nidaq.getIsConnected(), false);

//   nidaq.connect();
//   ASSERT_EQ(nidaq.getIsConnected(), true);

//   EXPECT_THROW(nidaq.connect(), devices::DeviceIsConnectedException);

//   nidaq.startRecording();
//   EXPECT_THROW(nidaq.disconnect(), devices::DeviceIsRecordingException);
//   nidaq.stopRecording();

//   nidaq.disconnect();
//   ASSERT_EQ(nidaq.getIsConnected(), false);

//   EXPECT_THROW(nidaq.disconnect(), devices::DeviceIsNotConnectedException);
// }

// TEST(Nidaq, recording) {
//   auto nidaq = devices::NidaqDevice(4, 1000);
//   ASSERT_EQ(nidaq.getIsRecording(), false);

//   EXPECT_THROW(nidaq.startRecording(),
//   devices::DeviceIsNotConnectedException);

//   nidaq.connect();
//   nidaq.startRecording();
//   ASSERT_EQ(nidaq.getIsRecording(), true);

//   EXPECT_THROW(nidaq.startRecording(), devices::DeviceIsRecordingException);

//   nidaq.stopRecording();
//   ASSERT_EQ(nidaq.getIsRecording(), false);

//   EXPECT_THROW(nidaq.stopRecording(),
//   devices::DeviceIsNotRecordingException);
// }

// TEST(Nidaq, callback) {
//   auto nidaq = devices::NidaqDevice(4, 1000);
//   nidaq.connect();
//   nidaq.startRecording();

//   bool callbackCalled = false;
//   auto callback = [&callbackCalled](const devices::data::DataPoint &newData)
//   {
//     callbackCalled = true;
//   };
//   nidaq.onNewData.listen(callback);

//   ASSERT_EQ(callbackCalled, false);
//   std::this_thread::sleep_for(std::chrono::milliseconds(50));
//   ASSERT_EQ(callbackCalled, true);
// }