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
  logger.giveTimeToUpdate();
  ASSERT_EQ(logger.count("The device DelsysEmgDevice is now connected"), 2);
  ASSERT_EQ(logger.count("The device MagstimRapidDevice is now connected"), 2);
  ASSERT_TRUE(logger.contains("All devices are now connected"));
  logger.clear();

  // Reconnect the devices even if they are already connected
  areConnected = devices.connect();
  ASSERT_TRUE(areConnected);
  ASSERT_TRUE(devices.getIsConnected());
  // Even though it is sync, the messages to the logger are sometimes late
  logger.giveTimeToUpdate();
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
  logger.giveTimeToUpdate();
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
  logger.giveTimeToUpdate();
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
  logger.giveTimeToUpdate();
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
  logger.giveTimeToUpdate();
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

TEST(Devices, StartDataStreaming) {
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

  // The system cannot start data streaming if it is not connected
  bool isStreamingData = devices.startDataStreaming();
  ASSERT_FALSE(isStreamingData);
  ASSERT_FALSE(devices.getIsStreamingData());
  ASSERT_EQ(logger.count("Cannot send a command to the device "
                         "DelsysCommandTcpDevice because it is not connected"),
            4); // Twice for starting and twice for stopping due to failed start
  ASSERT_EQ(logger.count("The data collector DelsysEmgDataCollector failed to "
                         "start streaming data"),
            2);
  ASSERT_EQ(logger.count("The data collector DelsysEmgDataCollector has "
                         "stopped streaming data"),
            2);
  ASSERT_TRUE(logger.contains("All devices have stopped streaming data"));
  ASSERT_TRUE(logger.contains("One or more devices failed to start streaming "
                              "data, stopping all devices"));
  logger.clear();

  // Connect the system and start data streaming
  devices.connect();
  isStreamingData = devices.startDataStreaming();
  ASSERT_TRUE(isStreamingData);
  ASSERT_TRUE(devices.getIsStreamingData());
  ASSERT_EQ(
      logger.count(
          "The data collector DelsysEmgDataCollector is now streaming data"),
      2);
  ASSERT_TRUE(logger.contains("All devices are now streaming data"));
  logger.clear();

  // The system cannot start streaming data if it is already streaming
  isStreamingData = devices.startDataStreaming();
  ASSERT_TRUE(isStreamingData);
  ASSERT_TRUE(devices.getIsStreamingData());
  ASSERT_EQ(logger.count("The data collector DelsysEmgDataCollector is already "
                         "streaming data"),
            2);
  ASSERT_TRUE(logger.contains("All devices are now streaming data"));
  logger.clear();

  // Stop streaming data
  bool isNotStreamingData = devices.stopDataStreaming();
  ASSERT_TRUE(isNotStreamingData);
  ASSERT_FALSE(devices.getIsStreamingData());
  ASSERT_EQ(logger.count("The data collector DelsysEmgDataCollector has "
                         "stopped streaming data"),
            2);
  ASSERT_TRUE(logger.contains("All devices have stopped streaming data"));
  logger.clear();

  // The system cannot stop streaming data if it is not streaming
  isNotStreamingData = devices.stopDataStreaming();
  ASSERT_TRUE(isNotStreamingData);
  ASSERT_FALSE(devices.getIsStreamingData());
  ASSERT_EQ(
      logger.count(
          "The data collector DelsysEmgDataCollector is not streaming data"),
      2);
  ASSERT_TRUE(logger.contains("All devices have stopped streaming data"));
  logger.clear();

  // Disconnect the system
  devices.disconnect();
}

TEST(Devices, AutoStopDataStreaming) {
  // The system auto stop streaming data when the object is destroyed
  auto logger = TestLogger();
  {
    auto devices = devices::Devices();

    // Add a bunch of devices
    devices.add(std::make_unique<devices::DelsysEmgDeviceMock>());
    devices.add(devices::MagstimRapidDeviceMock::findMagstimDevice());
    devices.add(std::make_unique<devices::DelsysEmgDeviceMock>());
    devices.add(devices::MagstimRapidDeviceMock::findMagstimDevice());

    // Connect the system and start streaming data
    devices.connect();
    devices.startDataStreaming();
  }
  ASSERT_EQ(logger.count("All devices have stopped streaming data"), 1);
  logger.clear();

  // The system auto stop if disconnect is called
  {
    auto devices = devices::Devices();

    // Add a bunch of devices
    devices.add(std::make_unique<devices::DelsysEmgDeviceMock>());
    devices.add(devices::MagstimRapidDeviceMock::findMagstimDevice());
    devices.add(std::make_unique<devices::DelsysEmgDeviceMock>());
    devices.add(devices::MagstimRapidDeviceMock::findMagstimDevice());

    // Connect the system and start streaming data
    devices.connect();
    devices.startDataStreaming();
    devices.disconnect();

    ASSERT_FALSE(devices.getIsStreamingData());
    ASSERT_EQ(logger.count("All devices are now disconnected"), 1);
  }
}

TEST(Devices, StartDataStreamingFailed) {
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
      .shouldFailToStartDataStreaming = true;

  devices.connect();
  bool isStreamingData = devices.startDataStreaming();
  ASSERT_FALSE(devices.getIsStreamingData());
  // Even though it is sync, the messages to the logger are sometimes late
  logger.giveTimeToUpdate();
  ASSERT_EQ(
      logger.count(
          "The data collector DelsysEmgDataCollector is now streaming data"),
      1);
  ASSERT_EQ(logger.count("The data collector DelsysEmgDataCollector failed to "
                         "start streaming data"),
            1);
  ASSERT_EQ(logger.count("The data collector DelsysEmgDataCollector has "
                         "stopped streaming data"),
            2);
  ASSERT_TRUE(logger.contains("One or more devices failed to start streaming "
                              "data, stopping all devices"));
  ASSERT_TRUE(logger.contains("All devices have stopped streaming data"));
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

  // The system cannot start recording data if it is not connected
  bool isRecording = devices.startRecording();
  ASSERT_FALSE(isRecording);
  ASSERT_FALSE(devices.getIsRecording());
  ASSERT_EQ(logger.count("The data collector DelsysEmgDataCollector is not "
                         "streaming data, so it cannot start recording"),
            2);
  ASSERT_EQ(logger.count(
                "The data collector DelsysEmgDataCollector is not recording"),
            2);
  ASSERT_TRUE(logger.contains("All devices have stopped recording"));
  ASSERT_TRUE(logger.contains("One or more devices failed to start "
                              "recording, stopping to record on all devices"));
  logger.clear();

  // Connect the system is still not enough to start recording
  devices.connect();
  isRecording = devices.startRecording();
  ASSERT_FALSE(isRecording);
  ASSERT_FALSE(devices.getIsRecording());
  ASSERT_EQ(logger.count("The data collector DelsysEmgDataCollector is not "
                         "streaming data, so it cannot start recording"),
            2);
  ASSERT_EQ(logger.count(
                "The data collector DelsysEmgDataCollector is not recording"),
            2);
  ASSERT_TRUE(logger.contains("All devices have stopped recording"));
  ASSERT_TRUE(logger.contains("One or more devices failed to start "
                              "recording, stopping to record on all devices"));

  logger.clear();

  // Start streaming data should allow to start recording
  devices.startDataStreaming();
  isRecording = devices.startRecording();
  ASSERT_TRUE(isRecording);
  ASSERT_TRUE(devices.getIsRecording());
  ASSERT_EQ(logger.count(
                "The data collector DelsysEmgDataCollector is now recording"),
            2);
  ASSERT_TRUE(logger.contains("All devices are now recording"));
  logger.clear();

  // The system cannot start recording data if it is already recording
  isRecording = devices.startRecording();
  ASSERT_TRUE(isRecording);
  ASSERT_TRUE(devices.getIsRecording());
  ASSERT_EQ(logger.count("The data collector DelsysEmgDataCollector is already "
                         "recording"),
            2);
  ASSERT_TRUE(logger.contains("All devices are now recording"));
  logger.clear();

  // Stop recording data
  bool isNotRecording = devices.stopRecording();
  ASSERT_TRUE(isNotRecording);
  ASSERT_FALSE(devices.getIsRecording());
  ASSERT_EQ(logger.count("The data collector DelsysEmgDataCollector has "
                         "stopped recording"),
            2);
  ASSERT_TRUE(logger.contains("All devices have stopped recording"));
  logger.clear();

  // The system cannot stop recording data if it is not recording
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

    // Connect the system and start streaming data
    devices.connect();
    devices.startDataStreaming();
    devices.startRecording();
  }
  ASSERT_EQ(logger.count("All devices have stopped recording"), 1);
  logger.clear();

  // The system auto stop if stop data streaming is called
  {
    auto devices = devices::Devices();

    // Add a bunch of devices
    devices.add(std::make_unique<devices::DelsysEmgDeviceMock>());
    devices.add(devices::MagstimRapidDeviceMock::findMagstimDevice());
    devices.add(std::make_unique<devices::DelsysEmgDeviceMock>());
    devices.add(devices::MagstimRapidDeviceMock::findMagstimDevice());

    // Connect the system and start streaming data
    devices.connect();
    devices.startDataStreaming();
    devices.startRecording();
    devices.stopDataStreaming();

    ASSERT_FALSE(devices.getIsRecording());
    ASSERT_EQ(logger.count("All devices have stopped recording"), 1);
    logger.clear();
  }

  // The system auto stop if disconnect is called
  {
    auto devices = devices::Devices();

    // Add a bunch of devices
    devices.add(std::make_unique<devices::DelsysEmgDeviceMock>());
    devices.add(devices::MagstimRapidDeviceMock::findMagstimDevice());
    devices.add(std::make_unique<devices::DelsysEmgDeviceMock>());
    devices.add(devices::MagstimRapidDeviceMock::findMagstimDevice());

    // Connect the system and start streaming data
    devices.connect();
    devices.startDataStreaming();
    devices.startRecording();
    devices.disconnect();

    ASSERT_FALSE(devices.getIsRecording());
    ASSERT_EQ(logger.count("All devices have stopped recording"), 1);
    logger.clear();
  }
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

  // Connect the system and start streaming data
  devices.connect();
  devices.startDataStreaming();
  devices.startRecording();

  // Clear the devices
  devices.clear();
  ASSERT_EQ(devices.size(), 0);
  ASSERT_FALSE(devices.getIsConnected());
  ASSERT_FALSE(devices.getIsStreamingData());
  ASSERT_FALSE(devices.getIsRecording());
  ASSERT_TRUE(logger.contains("All devices have stopped recording"));
  ASSERT_TRUE(logger.contains("All devices have stopped streaming data"));
  ASSERT_TRUE(logger.contains("All devices are now disconnected"));
}

TEST(Devices, LiveData) {
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

  // Connect the system and start streaming data should start the live data
  devices.connect();
  devices.startDataStreaming();
  auto now = std::chrono::system_clock::now();

  // All the time series should have the same starting time
  for (auto &[deviceId, dataCollector] : devices.getDataCollectors()) {
    auto data = data::TimeSeries(dataCollector->getSerializedLiveData());
    ASSERT_LE(data.getStartingTime().time_since_epoch(),
              now.time_since_epoch());
  }

  // Data are supposed to be collected in the live data
  std::map<size_t, size_t> sizes;
  for (auto &[deviceId, dataCollector] : devices.getDataCollectors()) {
    sizes[deviceId] =
        data::TimeSeries(dataCollector->getSerializedLiveData()).size();
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  for (auto &[deviceId, dataCollector] : devices.getDataCollectors()) {
    ASSERT_GT(data::TimeSeries(dataCollector->getSerializedLiveData()).size(),
              sizes[deviceId]);
  }

  // Stop streaming data should stop the live data
  devices.stopDataStreaming();
  sizes.clear();
  for (auto &[deviceId, dataCollector] : devices.getDataCollectors()) {
    sizes[deviceId] = dataCollector->getLiveData().size();
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  for (auto &[deviceId, dataCollector] : devices.getDataCollectors()) {
    ASSERT_EQ(dataCollector->getLiveData().size(), sizes[deviceId]);
  }
}

TEST(Devices, TrialData) {
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

  // Connect the system and start streaming data does not start the trial data
  devices.connect();
  devices.startDataStreaming();

  std::map<size_t, size_t> sizes;
  for (auto &[deviceId, dataCollector] : devices.getDataCollectors()) {
    sizes[deviceId] = dataCollector->getTrialData().size();
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  for (auto &[deviceId, dataCollector] : devices.getDataCollectors()) {
    ASSERT_EQ(dataCollector->getTrialData().size(), sizes[deviceId]);
  }

  devices.startRecording();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  auto now = std::chrono::system_clock::now();
  devices.stopRecording();
  // All the time series should have the same starting time
  for (auto &[deviceId, dataCollector] : devices.getDataCollectors()) {
    const auto &data = dataCollector->getTrialData();
    ASSERT_LE(data.getStartingTime().time_since_epoch(),
              now.time_since_epoch());
  }
  for (auto &[deviceId, dataCollector] : devices.getDataCollectors()) {
    ASSERT_GT(dataCollector->getTrialData().size(), sizes[deviceId]);
  }

  // Stopping the recording should stop the trial data
  sizes.clear();
  for (auto &[deviceId, dataCollector] : devices.getDataCollectors()) {
    sizes[deviceId] = dataCollector->getTrialData().size();
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  for (auto &[deviceId, dataCollector] : devices.getDataCollectors()) {
    ASSERT_EQ(dataCollector->getTrialData().size(), sizes[deviceId]);
  }
}

TEST(Devices, SerializeTrialData) {
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

  // Record some data
  devices.connect();
  devices.startDataStreaming();
  devices.startRecording();
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  devices.disconnect();

  // Serialize the data
  auto data = devices.getLastTrialDataSerialized();
  ASSERT_EQ(data.size(), 2);
  ASSERT_EQ(data[0]["name"], "DelsysEmgDataCollector");
  ASSERT_EQ(data[0]["data"]["startingTime"],
            devices.getDataCollector(deviceIds[0])
                .getTrialData()
                .getStartingTime()
                .time_since_epoch()
                .count());
  ASSERT_EQ(data[0]["data"]["data"].size(),
            devices.getDataCollector(deviceIds[0]).getTrialData().size());
  ASSERT_EQ(data[1]["name"], "DelsysEmgDataCollector");
  ASSERT_EQ(data[1]["data"]["startingTime"],
            devices.getDataCollector(deviceIds[2])
                .getTrialData()
                .getStartingTime()
                .time_since_epoch()
                .count());
  ASSERT_EQ(data[1]["data"]["data"].size(),
            devices.getDataCollector(deviceIds[2]).getTrialData().size());
}
