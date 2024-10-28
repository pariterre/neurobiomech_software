#include <gtest/gtest.h>
#include <iostream>
#include <thread>

#include "Devices/Concrete/DelsysEmgDevice.h"
#include "utils.h"

static double requiredPrecision(1e-6);
using namespace STIMWALKER_NAMESPACE;

int findDataOffset(const data::TimeSeries &data) {
  // The fake data are based on a sine wave but
  // offset by the number of data previously taken in any of the
  // test. We therefore search for this offset first, then it
  // should be a sine wave. The offset is found when a value is
  // exact and the next one is also exact (determining the
  // direction of the sine wave).
  int offset = 0;
  while (true) {
    float value = static_cast<float>(
        std::sin(static_cast<float>(offset) / 2000.0f * 2 * M_PI));
    float nextValue = static_cast<float>(
        std::sin(static_cast<float>(offset + 1) / 2000.0f * 2 * M_PI));
    if ((std::abs(data[0].getData()[0] - value) < requiredPrecision) &&
        (std::abs(data[1].getData()[0] - nextValue) < requiredPrecision)) {
      break;
    }
    if (offset > 2000) {
      // But we know for sure it will never be that far in (as the wave has
      // looped)
      return -1;
    }
    offset++;
  }
  return offset;
}

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
  logger.giveTimeToUpdate();
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

TEST(Delsys, StartRecording) {
  auto logger = TestLogger();
  auto delsys = devices::DelsysEmgDeviceMock();

  // Start the recording of data before connecting or streaming should fail
  bool isRecording = delsys.startRecording();
  ASSERT_FALSE(isRecording);
  ASSERT_FALSE(delsys.getIsRecording());
  // The logger is sometimes late
  logger.giveTimeToUpdate();
  ASSERT_TRUE(
      logger.contains("The data collector DelsysEmgDataCollector is not "
                      "streaming data, so it cannot start recording"));
  logger.clear();

  // Connect the system
  delsys.connect();
  isRecording = delsys.startRecording();
  ASSERT_FALSE(isRecording);
  ASSERT_FALSE(delsys.getIsRecording());
  // The logger is sometimes late
  logger.giveTimeToUpdate();
  ASSERT_TRUE(
      logger.contains("The data collector DelsysEmgDataCollector is "
                      "not streaming data, so it cannot start recording"));
  logger.clear();

  // Start streaming data
  delsys.startDataStreaming();
  isRecording = delsys.startRecording();
  ASSERT_TRUE(isRecording);
  ASSERT_TRUE(delsys.getIsRecording());
  // The logger is sometimes late
  logger.giveTimeToUpdate();
  ASSERT_TRUE(logger.contains("The data collector DelsysEmgDataCollector is "
                              "now recording"));
  logger.clear();

  // Stop recording
  bool isNotRecording = delsys.stopRecording();
  ASSERT_TRUE(isNotRecording);
  ASSERT_FALSE(delsys.getIsRecording());
  // The logger is sometimes late
  logger.giveTimeToUpdate();
  ASSERT_TRUE(logger.contains("The data collector DelsysEmgDataCollector has "
                              "stopped recording"));
  logger.clear();

  // The system cannot stop recording if it is not recording
  isNotRecording = delsys.stopRecording();
  ASSERT_TRUE(isNotRecording);
  ASSERT_FALSE(delsys.getIsRecording());
  // The logger is sometimes late
  logger.giveTimeToUpdate();
  ASSERT_TRUE(logger.contains("The data collector DelsysEmgDataCollector is "
                              "not recording"));
}

TEST(Delsys, AutoStopRecording) {
  // The system auto stop recording when the object is destroyed
  auto logger = TestLogger();
  {
    auto delsys = devices::DelsysEmgDeviceMock();
    delsys.connect();
    delsys.startDataStreaming();
    delsys.startRecording();
  }
  ASSERT_TRUE(logger.contains(
      "The data collector DelsysEmgDataCollector has stopped recording"));
  logger.clear();

  // The system auto stop if stop data streaming is called
  {
    auto delsys = devices::DelsysEmgDeviceMock();
    delsys.connect();
    delsys.startDataStreaming();
    delsys.startRecording();
    delsys.stopDataStreaming();

    ASSERT_FALSE(delsys.getIsRecording());
    ASSERT_TRUE(logger.contains("The data collector DelsysEmgDataCollector has "
                                "stopped recording"));
    logger.clear();
  }

  // The system auto stop if disconnect is called
  {
    auto delsys = devices::DelsysEmgDeviceMock();
    delsys.connect();
    delsys.startDataStreaming();
    delsys.startRecording();
    delsys.disconnect();

    ASSERT_FALSE(delsys.getIsRecording());
    ASSERT_TRUE(logger.contains("The data collector DelsysEmgDataCollector has "
                                "stopped recording"));
    logger.clear();
  }
}

TEST(Delsys, LiveData) {
  auto delsys = devices::DelsysEmgDeviceMock();
  delsys.connect();

  // Live data should collect even if the system is not recording
  delsys.startDataStreaming();
  ASSERT_FALSE(delsys.getIsRecording());
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  delsys.stopDataStreaming();

  // Get the data
  const auto &data = delsys.getLiveData();
  // Technically it should have recorded be exactly 200 (2000Hz). But the
  // material is not that precise. So we just check that it is at least 150
  ASSERT_GE(data.size(), 150);

  int offset = findDataOffset(data);
  if (offset == -1) {
    FAIL() << "Could not find the offset in the data";
  }

  for (size_t i = 0; i < data.size(); i++) {
    for (size_t j = 0; j < data[i].getData().size(); j++) {
      float value = static_cast<float>(
          std::sin((i + static_cast<size_t>(offset)) / 2000.0 * 2 * M_PI));
      ASSERT_NEAR(data[i].getData()[j], value, requiredPrecision);
    }
  }
}

TEST(Delsys, TrialData) {
  auto delsys = devices::DelsysEmgDeviceMock();
  delsys.connect();

  // Trial data should only collect when the system is recording
  delsys.startDataStreaming();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Get the data
  const auto &data = delsys.getTrialData();
  ASSERT_EQ(data.size(), 0);

  auto now = std::chrono::system_clock::now();
  delsys.startRecording();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  delsys.stopRecording();

  ASSERT_ALMOST_NOW(data.getStartingTime(), now);
  // Technically it should have recorded be exactly 200 (2000Hz). But the
  // material is not that precise. So we just check that it is at least 150
  ASSERT_GE(data.size(), 150);

  int offset = findDataOffset(data);
  if (offset == -1) {
    FAIL() << "Could not find the offset in the data";
  }

  for (size_t i = 0; i < data.size(); i++) {
    for (size_t j = 0; j < data[i].getData().size(); j++) {
      float value = static_cast<float>(
          std::sin((i + static_cast<size_t>(offset)) / 2000.0 * 2 * M_PI));
      ASSERT_NEAR(data[i].getData()[j], value, requiredPrecision);
    }
  }
}
