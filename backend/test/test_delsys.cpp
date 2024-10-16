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
  bool isConnected = delsys.connect();
  ASSERT_TRUE(isConnected);
  ASSERT_TRUE(delsys.getIsConnected());
  // The logger is sometimes late
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  ASSERT_TRUE(logger.contains("The device DelsysEmgDevice is now connected"));
  logger.clear();

  // Cannot connect twice
  isConnected = delsys.connect();
  ASSERT_TRUE(isConnected);
  ASSERT_TRUE(logger.contains("Cannot connect to the device DelsysEmgDevice "
                              "because it is already connected"));
  logger.clear();

  // Disconnecting, shows as not connected anymore
  bool isDisconnected = delsys.disconnect();
  ASSERT_TRUE(isDisconnected);
  ASSERT_FALSE(delsys.getIsConnected());
  ASSERT_TRUE(
      logger.contains("The device DelsysEmgDevice is now disconnected"));
  logger.clear();

  // Cannot disconnect twice
  isDisconnected = delsys.disconnect();
  ASSERT_TRUE(isDisconnected);
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
  bool isConnected = delsys.connect();
  ASSERT_FALSE(isConnected);
  ASSERT_FALSE(delsys.getIsConnected());
  ASSERT_TRUE(delsys.getHasFailedToConnect());
  ASSERT_TRUE(
      logger.contains("Could not connect to the device DelsysEmgDevice"));
}

TEST(Delsys, StartDataStreamingAsync) {
  auto logger = TestLogger();
  auto delsys = devices::DelsysEmgDeviceMock();

  delsys.connect();
  delsys.startDataStreamingAsync();
  ASSERT_FALSE(delsys.getIsStreamingData());

  // Wait for the data stream to start
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  ASSERT_TRUE(delsys.getIsStreamingData());
}

TEST(Delsys, StartDataStreamingFailedAsync) {
  auto logger = TestLogger();
  auto delsys = devices::DelsysEmgDeviceMock();
  delsys.shouldFailToStartDataStreaming = true;

  delsys.connect();
  delsys.startDataStreamingAsync();
  ASSERT_FALSE(delsys.getIsStreamingData());

  // Wait for the data stream to start
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  ASSERT_FALSE(delsys.getIsStreamingData());
  ASSERT_TRUE(delsys.getHasFailedToStartDataStreaming());
  ASSERT_TRUE(logger.contains("The data collector DelsysEmgDataCollector "
                              "failed to start streaming data"));
}

TEST(Delsys, StartDataStreaming) {
  auto logger = TestLogger();
  auto delsys = devices::DelsysEmgDeviceMock();

  // The system cannot start streaming data if it is not connected
  bool isStreamingData = delsys.startDataStreaming();
  ASSERT_FALSE(isStreamingData);
  ASSERT_FALSE(delsys.getIsStreamingData());
  ASSERT_TRUE(
      logger.contains("Cannot send a command to the device "
                      "DelsysCommandTcpDevice because it is not connected"));
  ASSERT_TRUE(logger.contains("The data collector DelsysEmgDataCollector "
                              "failed to start streaming data"));
  logger.clear();

  // Connect the system and start streaming data
  delsys.connect();
  isStreamingData = delsys.startDataStreaming();
  ASSERT_TRUE(isStreamingData);
  ASSERT_TRUE(delsys.getIsStreamingData());
  ASSERT_TRUE(logger.contains(
      "The data collector DelsysEmgDataCollector is now streaming data"));
  logger.clear();

  // The system cannot start streaming data if it is already streaming
  isStreamingData = delsys.startDataStreaming();
  ASSERT_TRUE(isStreamingData);
  ASSERT_TRUE(delsys.getIsStreamingData());
  ASSERT_TRUE(logger.contains(
      "The data collector DelsysEmgDataCollector is already streaming data"));
  logger.clear();

  // Stop streaming data
  bool isNotStreamingData = delsys.stopDataStreaming();
  ASSERT_TRUE(isNotStreamingData);
  ASSERT_FALSE(delsys.getIsStreamingData());
  ASSERT_TRUE(logger.contains(
      "The data collector DelsysEmgDataCollector has stopped streaming data"));
  logger.clear();

  // The system cannot stop streaming data if it is not streaming
  isNotStreamingData = delsys.stopDataStreaming();
  ASSERT_TRUE(isNotStreamingData);
  ASSERT_FALSE(delsys.getIsStreamingData());
  ASSERT_TRUE(logger.contains(
      "The data collector DelsysEmgDataCollector is not streaming data"));
  logger.clear();

  // Disconnect the system
  delsys.disconnect();
}

TEST(Delsys, AutoStopDataStreaming) {
  // The system auto stop streaming data when the object is destroyed
  auto logger = TestLogger();
  {
    auto delsys = devices::DelsysEmgDeviceMock();
    delsys.connect();
    delsys.startDataStreaming();
  }
  ASSERT_TRUE(logger.contains(
      "The data collector DelsysEmgDataCollector has stopped streaming data"));
  logger.clear();

  // The system auto stop if disconnect is called
  {
    auto delsys = devices::DelsysEmgDeviceMock();
    delsys.connect();
    delsys.startDataStreaming();
    delsys.disconnect();

    ASSERT_FALSE(delsys.getIsStreamingData());
    ASSERT_TRUE(logger.contains("The data collector DelsysEmgDataCollector has "
                                "stopped streaming data"));
  }
}

TEST(Delsys, StartDataStreamingFailed) {
  auto logger = TestLogger();
  auto delsys = devices::DelsysEmgDeviceMock();
  delsys.shouldFailToStartDataStreaming = true;

  delsys.connect();
  bool isStreamingData = delsys.startDataStreaming();
  ASSERT_FALSE(isStreamingData);
  ASSERT_FALSE(delsys.getIsStreamingData());
  ASSERT_TRUE(logger.contains("The data collector DelsysEmgDataCollector "
                              "failed to start streaming data"));
}

TEST(Delsys, PauseRecording) {
  auto logger = TestLogger();
  auto delsys = devices::DelsysEmgDeviceMock();

  // Pause the system prior to recording
  delsys.pauseRecording();
  ASSERT_TRUE(delsys.getIsPaused());

  // Start the recording
  delsys.connect();
  delsys.startDataStreaming();

  // Wait for a bit. There should not be any data
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  ASSERT_EQ(delsys.getTimeSeries().size(), 0);

  // Resume the recording
  delsys.resumeRecording();
  ASSERT_FALSE(delsys.getIsPaused());

  // Wait for a bit. There should be data
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  size_t dataCount = delsys.getTimeSeries().size();
  ASSERT_GT(dataCount, 0);

  // Pause the recording again
  delsys.pauseRecording();
  ASSERT_TRUE(delsys.getIsPaused());

  // Wait for a bit. There should not be any new data
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  ASSERT_EQ(delsys.getTimeSeries().size(), dataCount);
}

TEST(Delsys, Data) {
  auto delsys = devices::DelsysEmgDeviceMock();
  delsys.connect();

  // Wait for the data to be collected
  delsys.startDataStreaming();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  delsys.stopDataStreaming();

  // Get the data
  const auto &data = delsys.getTimeSeries();
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
