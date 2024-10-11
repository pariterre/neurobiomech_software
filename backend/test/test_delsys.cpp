#include <gtest/gtest.h>
#include <iostream>
#include <thread>

#include "Devices/Concrete/DelsysEmgDevice.h"
#include "utils.h"

static double requiredPrecision(1e-6);

using namespace STIMWALKER_NAMESPACE;

TEST(Delsys, Info) {
  auto delsys = devices::DelsysEmgDeviceMock();

  ASSERT_STREQ(delsys.deviceName().c_str(), "DelsysEmgDevice");
}

TEST(Delsys, ConnectAsync) {
  auto logger = TestLogger();
  auto delsys = devices::DelsysEmgDeviceMock();

  delsys.connectAsync();
  ASSERT_FALSE(delsys.getIsConnected());

  // Wait for the connection to be established
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  ASSERT_TRUE(delsys.getIsConnected());
}

TEST(Delsys, ConnectFailedAsync) {
  auto logger = TestLogger();
  auto delsys = devices::DelsysEmgDeviceMock();

  delsys.shouldFailToConnect = true;
  delsys.connectAsync();
  ASSERT_FALSE(delsys.getIsConnected());

  // Wait for the connection to be established
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  ASSERT_FALSE(delsys.getIsConnected());
  ASSERT_TRUE(delsys.getHasFailedToConnect());
  ASSERT_TRUE(
      logger.contains("Could not connect to the device DelsysEmgDevice"));
}

TEST(Delsys, Connect) {
  auto logger = TestLogger();
  auto delsys = devices::DelsysEmgDeviceMock();

  // Is not connected when created
  ASSERT_FALSE(delsys.getIsConnected());

  // Connect to the device, now shows as connected
  delsys.connect();
  ASSERT_TRUE(delsys.getIsConnected());
  ASSERT_TRUE(logger.contains("The device DelsysEmgDevice is now connected"));
  logger.clear();

  // Cannot connect twice
  delsys.connect();
  ASSERT_TRUE(logger.contains("Cannot connect to the device DelsysEmgDevice "
                              "because it is already connected"));
  logger.clear();

  // Disconnecting, shows as not connected anymore
  delsys.disconnect();
  ASSERT_FALSE(delsys.getIsConnected());
  ASSERT_TRUE(
      logger.contains("The device DelsysEmgDevice is now disconnected"));
  logger.clear();

  // Cannot disconnect twice
  delsys.disconnect();
  ASSERT_TRUE(logger.contains(
      "Cannot disconnect from the device DelsysEmgDevice because "
      "it is not connected"));
  logger.clear();
}

TEST(Delsys, AutoDisconnect) {
  // The Delsys disconnect automatically when the object is destroyed
  auto logger = TestLogger();
  {
    auto delsys = devices::DelsysEmgDeviceMock();
    delsys.connect();
  }
  ASSERT_TRUE(
      logger.contains("The device DelsysEmgDevice is now disconnected"));
  logger.clear();
}

TEST(Delsys, ConnectFailed) {
  auto logger = TestLogger();
  auto delsys = devices::DelsysEmgDeviceMock();

  delsys.shouldFailToConnect = true;
  delsys.connect();
  ASSERT_FALSE(delsys.getIsConnected());
  ASSERT_TRUE(delsys.getHasFailedToConnect());
  ASSERT_TRUE(
      logger.contains("Could not connect to the device DelsysEmgDevice"));
}

TEST(Delsys, StartRecordingAsync) {
  auto logger = TestLogger();
  auto delsys = devices::DelsysEmgDeviceMock();

  delsys.connect();
  delsys.startRecordingAsync();
  ASSERT_FALSE(delsys.getIsRecording());

  // Wait for the recording to start
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  ASSERT_TRUE(delsys.getIsRecording());
}

TEST(Delsys, StartRecordingFailedAsync) {
  auto logger = TestLogger();
  auto delsys = devices::DelsysEmgDeviceMock();

  delsys.connect();
  delsys.shouldFailToStartRecording = true;
  delsys.startRecordingAsync();
  ASSERT_FALSE(delsys.getIsRecording());

  // Wait for the recording to start
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  ASSERT_FALSE(delsys.getIsRecording());
  ASSERT_TRUE(delsys.getHasFailedToStartRecording());
  ASSERT_TRUE(logger.contains(
      "The data collector DelsysEmgDataCollector failed to start recording"));
}

TEST(Delsys, StartRecording) {
  auto logger = TestLogger();
  auto delsys = devices::DelsysEmgDeviceMock();

  // The system cannot start recording if it is not connected
  delsys.startRecording();
  ASSERT_FALSE(delsys.getIsRecording());
  ASSERT_TRUE(
      logger.contains("Cannot send a command to the device "
                      "DelsysCommandTcpDevice because it is not connected"));
  ASSERT_TRUE(logger.contains(
      "The data collector DelsysEmgDataCollector failed to start recording"));
  logger.clear();

  // Connect the system and start recording
  delsys.connect();
  delsys.startRecording();
  ASSERT_TRUE(delsys.getIsRecording());
  ASSERT_TRUE(logger.contains(
      "The data collector DelsysEmgDataCollector is now recording"));
  logger.clear();

  // The system cannot start recording if it is already recording
  delsys.startRecording();
  ASSERT_TRUE(delsys.getIsRecording());
  ASSERT_TRUE(logger.contains(
      "The data collector DelsysEmgDataCollector is already recording"));
  logger.clear();

  // Stop recording
  delsys.stopRecording();
  ASSERT_FALSE(delsys.getIsRecording());
  ASSERT_TRUE(logger.contains(
      "The data collector DelsysEmgDataCollector has stopped recording"));
  logger.clear();

  // The system cannot stop recording if it is not recording
  delsys.stopRecording();
  ASSERT_FALSE(delsys.getIsRecording());
  ASSERT_TRUE(logger.contains(
      "The data collector DelsysEmgDataCollector is not recording"));
  logger.clear();

  // Disconnect the system
  delsys.disconnect();
}

TEST(Delsys, AutoStopRecording) {
  // The system auto stop recording when the object is destroyed
  auto logger = TestLogger();
  {
    auto delsys = devices::DelsysEmgDeviceMock();
    delsys.connect();
    delsys.startRecording();
  }
  ASSERT_TRUE(logger.contains(
      "The data collector DelsysEmgDataCollector has stopped recording"));
  logger.clear();

  // The system auto stop if disconnect is called
  {
    auto delsys = devices::DelsysEmgDeviceMock();
    delsys.connect();
    delsys.startRecording();
    delsys.disconnect();

    ASSERT_FALSE(delsys.getIsRecording());
    ASSERT_TRUE(logger.contains(
        "The data collector DelsysEmgDataCollector has stopped recording"));
  }
}

TEST(Delsys, StartRecordingFailed) {
  auto logger = TestLogger();
  auto delsys = devices::DelsysEmgDeviceMock();

  delsys.shouldFailToStartRecording = true;
  delsys.connect();
  delsys.startRecording();
  ASSERT_FALSE(delsys.getIsRecording());
  ASSERT_TRUE(logger.contains(
      "The data collector DelsysEmgDataCollector failed to start recording"));
}

TEST(Delsys, Data) {
  auto delsys = devices::DelsysEmgDeviceMock();
  delsys.connect();

  // Wait for the data to be collected
  delsys.startRecording();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  delsys.stopRecording();

  // Get the data
  const auto &data = delsys.getTrialData();
  // Technically it should have recorded be exactly 200 (2000Hz). But the
  // material is not that precise. So we just check that it is at least 150
  ASSERT_GE(data.size(), 150);

  // Check the data. The fake data are based on a sine wave but offset by the
  // number of data previously taken in any of the test. We therefore search for
  // this offset first, then it should be a sine wave. The offset is found when
  // a value is exact and the next one is also exact (determining the direction
  // of the sine wave).
  size_t offset = 0;
  while (true) {
    float value = static_cast<float>(
        std::sin(static_cast<float>(offset) / 2000.0f * 2 * M_PI));
    float nextValue = static_cast<float>(
        std::sin(static_cast<float>(offset + 1) / 2000.0f * 2 * M_PI));
    if ((std::abs(data[0].second[0] - value) < requiredPrecision) &&
        (std::abs(data[1].second[0] - nextValue) < requiredPrecision)) {
      break;
    }
    if (offset > 2000) {
      // But we know for sure it will never be that far in (as the wave has
      // looped)
      FAIL() << "Could not find the offset in the data";
    }
    offset++;
  }
  for (size_t i = 0; i < data.size(); i++) {
    for (size_t j = 0; j < data[i].second.size(); j++) {
      float value =
          static_cast<float>(std::sin((i + offset) / 2000.0 * 2 * M_PI));
      ASSERT_NEAR(data[i].second[j], value, requiredPrecision);
    }
  }
}
