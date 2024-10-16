#include <gtest/gtest.h>
#include <iostream>
#include <thread>

#include "Devices/all.h"
#include "utils.h"

using namespace STIMWALKER_NAMESPACE;

TEST(Devices, Add) {
  auto logger = TestLogger();
  auto devices = devices::Devices();

  // Add a bunch of devices
  std::vector<size_t> deviceIds;
  deviceIds.push_back(
      devices.add(std::make_unique<devices::DelsysEmgDeviceMock>()));
  deviceIds.push_back(
      devices.add(devices::MagstimRapidDeviceMock::findMagstimDevice()));
  deviceIds.push_back(
      devices.add(std::make_unique<devices::DelsysEmgDeviceMock>()));
  deviceIds.push_back(
      devices.add(devices::MagstimRapidDeviceMock::findMagstimDevice()));

  // Check that they were added correctly
  ASSERT_EQ(devices.size(), 4);
  ASSERT_EQ(devices[deviceIds[0]].deviceName(), "DelsysEmgDevice");
  ASSERT_EQ(devices[deviceIds[1]].deviceName(), "MagstimRapidDevice");
  ASSERT_EQ(devices[deviceIds[2]].deviceName(), "DelsysEmgDevice");
  ASSERT_EQ(devices[deviceIds[3]].deviceName(), "MagstimRapidDevice");

  // Remove one of the devices
  devices.remove(1);
  ASSERT_EQ(devices.size(), 3);
  ASSERT_EQ(devices[deviceIds[0]].deviceName(), "DelsysEmgDevice");
  ASSERT_EQ(devices[deviceIds[2]].deviceName(), "DelsysEmgDevice");
  ASSERT_EQ(devices[deviceIds[3]].deviceName(), "MagstimRapidDevice");

  // Remove all the devices
  devices.clear();
  ASSERT_EQ(devices.size(), 0);
}

TEST(Devices, Get) {
  auto logger = TestLogger();
  auto devices = devices::Devices();

  // Add a bunch of devices
  std::vector<size_t> deviceIds;
  deviceIds.push_back(
      devices.add(std::make_unique<devices::DelsysEmgDeviceMock>()));
  deviceIds.push_back(
      devices.add(devices::MagstimRapidDeviceMock::findMagstimDevice()));
  deviceIds.push_back(
      devices.add(std::make_unique<devices::DelsysEmgDeviceMock>()));
  deviceIds.push_back(
      devices.add(devices::MagstimRapidDeviceMock::findMagstimDevice()));

  // Ids are different for each device even accross different runs
  ASSERT_NE(deviceIds[0], 0);
  ASSERT_NE(deviceIds[0], deviceIds[1]);
  ASSERT_EQ(devices[deviceIds[0]].deviceName(), "DelsysEmgDevice");
  ASSERT_EQ(devices[deviceIds[1]].deviceName(), "MagstimRapidDevice");
  ASSERT_EQ(devices[deviceIds[2]].deviceName(), "DelsysEmgDevice");
  ASSERT_EQ(devices[deviceIds[3]].deviceName(), "MagstimRapidDevice");

  // getDevice and operator[] should return the same device
  ASSERT_EQ(&devices.getDevice(deviceIds[0]), &devices[deviceIds[0]]);
  ASSERT_EQ(&devices.getDevice(deviceIds[1]), &devices[deviceIds[1]]);
  ASSERT_EQ(&devices.getDevice(deviceIds[2]), &devices[deviceIds[2]]);
  ASSERT_EQ(&devices.getDevice(deviceIds[3]), &devices[deviceIds[3]]);

  // Same for getDataCollector
  ASSERT_EQ(
      &devices.getDataCollector(deviceIds[0]),
      dynamic_cast<const devices::DataCollector *>(&devices[deviceIds[0]]));
  ASSERT_EQ(
      &devices.getDataCollector(deviceIds[2]),
      dynamic_cast<const devices::DataCollector *>(&devices[deviceIds[2]]));

  // Requesting an inexistent device should throw an exception
  ASSERT_THROW(devices[deviceIds.back() + 1], devices::DeviceNotFoundException);
  ASSERT_THROW(devices.getDevice(deviceIds.back() + 1),
               devices::DeviceNotFoundException);
  ASSERT_THROW(devices.getDataCollector(deviceIds.back() + 1),
               devices::DeviceNotFoundException);

  // Requesting an existent device which is not a data collector should throw
  // an exception
  ASSERT_THROW(devices.getDataCollector(deviceIds[1]),
               devices::DeviceNotFoundException);
  ASSERT_THROW(devices.getDataCollector(deviceIds[3]),
               devices::DeviceNotFoundException);
}

TEST(Devices, Connect) {
  auto logger = TestLogger();
  auto devices = devices::Devices();

  // Add a bunch of devices
  devices.add(std::make_unique<devices::DelsysEmgDeviceMock>());
  devices.add(devices::MagstimRapidDeviceMock::findMagstimDevice());
  devices.add(std::make_unique<devices::DelsysEmgDeviceMock>());
  devices.add(devices::MagstimRapidDeviceMock::findMagstimDevice());

  // Connect the devices
  bool areConnected = devices.connect();
  ASSERT_TRUE(areConnected);
  ASSERT_TRUE(devices.getIsConnected());
  // Even though it is sync, the messages to the logger are sometimes late
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  ASSERT_EQ(logger.count("The device DelsysEmgDevice is now connected"), 2);
  ASSERT_EQ(logger.count("The device MagstimRapidDevice is now connected"), 2);
  ASSERT_TRUE(logger.contains("All devices are now connected"));
  logger.clear();

  // Reconnect the devices even if they are already connected
  areConnected = devices.connect();
  ASSERT_TRUE(areConnected);
  ASSERT_TRUE(devices.getIsConnected());
  // Even though it is sync, the messages to the logger are sometimes late
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  ASSERT_EQ(logger.count("Cannot connect to the device DelsysEmgDevice because "
                         "it is already connected"),
            2);
  ASSERT_EQ(logger.count("Cannot connect to the device MagstimRapidDevice "
                         "because it is already connected"),
            2);
  ASSERT_TRUE(logger.contains("All devices are now connected"));
  logger.clear();

  // Disconnecting, shows as not connected anymore
  bool areDisconnected = devices.disconnect();
  ASSERT_TRUE(areDisconnected);
  ASSERT_FALSE(devices.getIsConnected());
  // Even though it is sync, the messages to the logger are sometimes late
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  ASSERT_EQ(logger.count("The device DelsysEmgDevice is now disconnected"), 2);
  ASSERT_EQ(logger.count("The device MagstimRapidDevice is now disconnected"),
            2);
  ASSERT_TRUE(logger.contains("All devices are now disconnected"));
  logger.clear();

  // Cannot disconnect twice
  areDisconnected = devices.disconnect();
  ASSERT_TRUE(areDisconnected);
  ASSERT_FALSE(devices.getIsConnected());
  // Even though it is sync, the messages to the logger are sometimes late
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  ASSERT_EQ(logger.count("Cannot disconnect from the device DelsysEmgDevice "
                         "because it is not connected"),
            2);
  ASSERT_EQ(logger.count("Cannot disconnect from the device MagstimRapidDevice "
                         "because it is not connected"),
            2);
  ASSERT_TRUE(logger.contains("All devices are now disconnected"));
  logger.clear();
}

TEST(Devices, AutoDisconnect) {
  // The Delsys disconnect automatically when the object is destroyed
  auto logger = TestLogger();
  {
    auto devices = devices::Devices();
    devices.connect();
  }
  // Even though it is sync, the messages to the logger are sometimes late
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  ASSERT_TRUE(logger.contains("All devices are now disconnected"));
  logger.clear();
}

TEST(Devices, ConnectFailed) {
  auto logger = TestLogger();
  auto devices = devices::Devices();

  // Add a bunch of devices
  std::vector<size_t> deviceIds;
  deviceIds.push_back(
      devices.add(std::make_unique<devices::DelsysEmgDeviceMock>()));
  deviceIds.push_back(
      devices.add(devices::MagstimRapidDeviceMock::findMagstimDevice()));
  deviceIds.push_back(
      devices.add(std::make_unique<devices::DelsysEmgDeviceMock>()));
  deviceIds.push_back(
      devices.add(devices::MagstimRapidDeviceMock::findMagstimDevice()));

  // Make the second Delsys fail to connect
  auto &delsys = const_cast<devices::Device &>(devices[deviceIds[2]]);
  dynamic_cast<devices::DelsysEmgDeviceMock &>(delsys).shouldFailToConnect =
      true;

  bool isConnected = devices.connect();
  ASSERT_FALSE(isConnected);
  ASSERT_FALSE(devices.getIsConnected());
  // Even though it is sync, the messages to the logger are sometimes late
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  ASSERT_EQ(logger.count("The device DelsysEmgDevice is now connected"), 1);
  ASSERT_EQ(logger.count("The device MagstimRapidDevice is now connected"), 2);
  ASSERT_EQ(logger.count("Could not connect to the device DelsysEmgDevice"), 1);
  ASSERT_EQ(logger.count("The device DelsysEmgDevice is now disconnected"), 1);
  ASSERT_EQ(logger.count("Cannot disconnect from the device DelsysEmgDevice "
                         "because it is not connected"),
            1);
  ASSERT_EQ(logger.count("The device MagstimRapidDevice is now disconnected"),
            2);
  ASSERT_TRUE(logger.contains(
      "One or more devices failed to connect, disconnecting all devices"));
}

TEST(Devices, StartRecording) {
  auto logger = TestLogger();
  auto devices = devices::Devices();

  // Add a bunch of devices
  std::vector<size_t> deviceIds;
  deviceIds.push_back(
      devices.add(std::make_unique<devices::DelsysEmgDeviceMock>()));
  deviceIds.push_back(
      devices.add(devices::MagstimRapidDeviceMock::findMagstimDevice()));
  deviceIds.push_back(
      devices.add(std::make_unique<devices::DelsysEmgDeviceMock>()));
  deviceIds.push_back(
      devices.add(devices::MagstimRapidDeviceMock::findMagstimDevice()));

  // The system cannot start recording if it is not connected
  bool isRecording = devices.startRecording();
  ASSERT_FALSE(isRecording);
  ASSERT_FALSE(devices.getIsRecording());
  ASSERT_EQ(logger.count("Cannot send a command to the device "
                         "DelsysCommandTcpDevice because it is not connected"),
            4); // Twice for starting and twice for stopping due to failed start
  ASSERT_EQ(logger.count("The data collector DelsysEmgDataCollector failed to "
                         "start recording"),
            2);
  ASSERT_EQ(
      logger.count(
          "The data collector DelsysEmgDataCollector has stopped recording"),
      2);
  ASSERT_TRUE(logger.contains("All devices have stopped recording"));
  ASSERT_TRUE(logger.contains(
      "One or more devices failed to start recording, stopping all devices"));
  logger.clear();

  // Connect the system and start recording
  devices.connect();
  isRecording = devices.startRecording();
  ASSERT_TRUE(isRecording);
  ASSERT_TRUE(devices.getIsRecording());
  ASSERT_EQ(logger.count(
                "The data collector DelsysEmgDataCollector is now recording"),
            2);
  ASSERT_TRUE(logger.contains("All devices are now recording"));
  logger.clear();

  // The system cannot start recording if it is already recording
  isRecording = devices.startRecording();
  ASSERT_TRUE(isRecording);
  ASSERT_TRUE(devices.getIsRecording());
  ASSERT_EQ(
      logger.count(
          "The data collector DelsysEmgDataCollector is already recording"),
      2);
  ASSERT_TRUE(logger.contains("All devices are now recording"));
  logger.clear();

  // Stop recording
  bool isNotRecording = devices.stopRecording();
  ASSERT_TRUE(isNotRecording);
  ASSERT_FALSE(devices.getIsRecording());
  ASSERT_EQ(
      logger.count(
          "The data collector DelsysEmgDataCollector has stopped recording"),
      2);
  ASSERT_TRUE(logger.contains("All devices have stopped recording"));
  logger.clear();

  // The system cannot stop recording if it is not recording
  isNotRecording = devices.stopRecording();
  ASSERT_TRUE(isNotRecording);
  ASSERT_FALSE(devices.getIsRecording());
  ASSERT_EQ(logger.count(
                "The data collector DelsysEmgDataCollector is not recording"),
            2);
  ASSERT_TRUE(logger.contains("All devices have stopped recording"));
  logger.clear();

  // Disconnect the system
  devices.disconnect();
}

TEST(Devices, AutoStopRecording) {
  // The system auto stop recording when the object is destroyed
  auto logger = TestLogger();
  {
    auto devices = devices::Devices();

    // Add a bunch of devices
    devices.add(std::make_unique<devices::DelsysEmgDeviceMock>());
    devices.add(devices::MagstimRapidDeviceMock::findMagstimDevice());
    devices.add(std::make_unique<devices::DelsysEmgDeviceMock>());
    devices.add(devices::MagstimRapidDeviceMock::findMagstimDevice());

    // Connect the system and start recording
    devices.connect();
    devices.startRecording();
  }
  ASSERT_EQ(logger.count("All devices have stopped recording"), 1);
  logger.clear();

  // The system auto stop if disconnect is called
  {
    auto devices = devices::Devices();

    // Add a bunch of devices
    devices.add(std::make_unique<devices::DelsysEmgDeviceMock>());
    devices.add(devices::MagstimRapidDeviceMock::findMagstimDevice());
    devices.add(std::make_unique<devices::DelsysEmgDeviceMock>());
    devices.add(devices::MagstimRapidDeviceMock::findMagstimDevice());

    // Connect the system and start recording
    devices.connect();
    devices.startRecording();
    devices.disconnect();

    ASSERT_FALSE(devices.getIsRecording());
    ASSERT_EQ(logger.count("All devices are now disconnected"), 1);
  }
}

TEST(Devices, StartRecordingFailed) {
  auto logger = TestLogger();
  auto devices = devices::Devices();

  // Add a bunch of devices
  std::vector<size_t> deviceIds;
  deviceIds.push_back(
      devices.add(std::make_unique<devices::DelsysEmgDeviceMock>()));
  deviceIds.push_back(
      devices.add(devices::MagstimRapidDeviceMock::findMagstimDevice()));
  deviceIds.push_back(
      devices.add(std::make_unique<devices::DelsysEmgDeviceMock>()));
  deviceIds.push_back(
      devices.add(devices::MagstimRapidDeviceMock::findMagstimDevice()));

  // Make the second Delsys fail to connect
  auto &delsys = const_cast<devices::Device &>(devices[deviceIds[2]]);
  dynamic_cast<devices::DelsysEmgDeviceMock &>(delsys)
      .shouldFailToStartRecording = true;

  devices.connect();
  bool isRecording = devices.startRecording();
  ASSERT_FALSE(devices.getIsRecording());
  // Even though it is sync, the messages to the logger are sometimes late
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  ASSERT_EQ(logger.count(
                "The data collector DelsysEmgDataCollector is now recording"),
            1);
  ASSERT_EQ(logger.count("The data collector DelsysEmgDataCollector failed to "
                         "start recording"),
            1);
  ASSERT_EQ(
      logger.count(
          "The data collector DelsysEmgDataCollector has stopped recording"),
      2);
  ASSERT_TRUE(logger.contains(
      "One or more devices failed to start recording, stopping all devices"));
  ASSERT_TRUE(logger.contains("All devices have stopped recording"));
}

TEST(Devices, Clear) {
  auto logger = TestLogger();
  auto devices = devices::Devices();

  // Add a bunch of devices
  std::vector<size_t> deviceIds;
  deviceIds.push_back(
      devices.add(std::make_unique<devices::DelsysEmgDeviceMock>()));
  deviceIds.push_back(
      devices.add(devices::MagstimRapidDeviceMock::findMagstimDevice()));
  deviceIds.push_back(
      devices.add(std::make_unique<devices::DelsysEmgDeviceMock>()));
  deviceIds.push_back(
      devices.add(devices::MagstimRapidDeviceMock::findMagstimDevice()));

  // Connect the system and start recording
  devices.connect();
  devices.startRecording();

  // Clear the devices
  devices.clear();
  ASSERT_EQ(devices.size(), 0);
  ASSERT_FALSE(devices.getIsConnected());
  ASSERT_FALSE(devices.getIsRecording());
  ASSERT_TRUE(logger.contains("All devices have stopped recording"));
  ASSERT_TRUE(logger.contains("All devices are now disconnected"));
}

TEST(Devices, Data) {
  auto logger = TestLogger();
  auto devices = devices::Devices();

  // Add a bunch of devices
  std::vector<size_t> deviceIds;
  deviceIds.push_back(
      devices.add(std::make_unique<devices::DelsysEmgDeviceMock>()));
  deviceIds.push_back(
      devices.add(devices::MagstimRapidDeviceMock::findMagstimDevice()));
  deviceIds.push_back(
      devices.add(std::make_unique<devices::DelsysEmgDeviceMock>()));
  deviceIds.push_back(
      devices.add(devices::MagstimRapidDeviceMock::findMagstimDevice()));

  // Connect the system and start recording
  devices.connect();
  devices.startRecording();
  auto now = std::chrono::system_clock::now();

  // All the time series should have the same starting time
  for (auto &[deviceId, dataCollector] : devices.getDataCollectors()) {
    auto timeSeries = dataCollector->getTrialData();
    ASSERT_LE(timeSeries.getStartingTime().time_since_epoch(),
              now.time_since_epoch());
  }
  logger.clear();

  // Pause the recording should pause the time series
  devices.pauseRecording();
  ASSERT_TRUE(devices.getIsPaused());
  ASSERT_TRUE(logger.contains("All devices have paused recording"));

  std::map<size_t, size_t> sizes;
  for (auto &[deviceId, dataCollector] : devices.getDataCollectors()) {
    sizes[deviceId] = dataCollector->getTrialData().size();
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  for (auto &[deviceId, dataCollector] : devices.getDataCollectors()) {
    ASSERT_EQ(dataCollector->getTrialData().size(), sizes[deviceId]);
  }

  // Resume the recording should resume the time series
  devices.resumeRecording();
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  for (auto &[deviceId, dataCollector] : devices.getDataCollectors()) {
    ASSERT_GT(dataCollector->getTrialData().size(), sizes[deviceId]);
  }
}
