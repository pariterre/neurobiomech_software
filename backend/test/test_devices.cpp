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
  std::vector<int> deviceIds;
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
  std::vector<int> deviceIds;
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
  ASSERT_THROW(devices[deviceIds[4]], devices::DeviceNotFoundException);
  ASSERT_THROW(devices.getDevice(deviceIds[4]),
               devices::DeviceNotFoundException);
  ASSERT_THROW(devices.getDataCollector(deviceIds[4]),
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
  std::vector<int> deviceIds;
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

// TEST(Delsys, StartRecording) {
//   auto logger = TestLogger();
//   auto delsys = devices::DelsysEmgDeviceMock();

//   // The system cannot start recording if it is not connected
//   bool isRecording = delsys.startRecording();
//   ASSERT_FALSE(isRecording);
//   ASSERT_FALSE(delsys.getIsRecording());
//   ASSERT_TRUE(
//       logger.contains("Cannot send a command to the device "
//                       "DelsysCommandTcpDevice because it is not connected"));
//   ASSERT_TRUE(logger.contains(
//       "The data collector DelsysEmgDataCollector failed to start
//       recording"));
//   logger.clear();

//   // Connect the system and start recording
//   delsys.connect();
//   isRecording = delsys.startRecording();
//   ASSERT_TRUE(isRecording);
//   ASSERT_TRUE(delsys.getIsRecording());
//   ASSERT_TRUE(logger.contains(
//       "The data collector DelsysEmgDataCollector is now recording"));
//   logger.clear();

//   // The system cannot start recording if it is already recording
//   isRecording = delsys.startRecording();
//   ASSERT_TRUE(isRecording);
//   ASSERT_TRUE(delsys.getIsRecording());
//   ASSERT_TRUE(logger.contains(
//       "The data collector DelsysEmgDataCollector is already recording"));
//   logger.clear();

//   // Stop recording
//   bool isNotRecording = delsys.stopRecording();
//   ASSERT_TRUE(isNotRecording);
//   ASSERT_FALSE(delsys.getIsRecording());
//   ASSERT_TRUE(logger.contains(
//       "The data collector DelsysEmgDataCollector has stopped recording"));
//   logger.clear();

//   // The system cannot stop recording if it is not recording
//   isNotRecording = delsys.stopRecording();
//   ASSERT_TRUE(isNotRecording);
//   ASSERT_FALSE(delsys.getIsRecording());
//   ASSERT_TRUE(logger.contains(
//       "The data collector DelsysEmgDataCollector is not recording"));
//   logger.clear();

//   // Disconnect the system
//   delsys.disconnect();
// }

// TEST(Delsys, AutoStopRecording) {
//   // The system auto stop recording when the object is destroyed
//   auto logger = TestLogger();
//   {
//     auto delsys = devices::DelsysEmgDeviceMock();
//     delsys.connect();
//     delsys.startRecording();
//   }
//   ASSERT_TRUE(logger.contains(
//       "The data collector DelsysEmgDataCollector has stopped recording"));
//   logger.clear();

//   // The system auto stop if disconnect is called
//   {
//     auto delsys = devices::DelsysEmgDeviceMock();
//     delsys.connect();
//     delsys.startRecording();
//     delsys.disconnect();

//     ASSERT_FALSE(delsys.getIsRecording());
//     ASSERT_TRUE(logger.contains(
//         "The data collector DelsysEmgDataCollector has stopped recording"));
//   }
// }

// TEST(Delsys, StartRecordingFailed) {
//   auto logger = TestLogger();
//   auto delsys = devices::DelsysEmgDeviceMock();
//   delsys.shouldFailToStartRecording = true;

//   delsys.connect();
//   bool isRecording = delsys.startRecording();
//   ASSERT_FALSE(isRecording);
//   ASSERT_FALSE(delsys.getIsRecording());
//   ASSERT_TRUE(logger.contains(
//       "The data collector DelsysEmgDataCollector failed to start
//       recording"));
// }

// TEST(Delsys, Data) {
//   auto delsys = devices::DelsysEmgDeviceMock();
//   delsys.connect();

//   // Wait for the data to be collected
//   bool isRecording = delsys.startRecording();
//   ASSERT_TRUE(isRecording);
//   std::this_thread::sleep_for(std::chrono::milliseconds(100));
//   delsys.stopRecording();

//   // Get the data
//   const auto &data = delsys.getTrialData();
//   // Technically it should have recorded be exactly 200 (2000Hz). But the
//   // material is not that precise. So we just check that it is at least 150
//   ASSERT_GE(data.size(), 150);

//   // Check the data. The fake data are based on a sine wave but offset by the
//   // number of data previously taken in any of the test. We therefore search
//   for
//   // this offset first, then it should be a sine wave. The offset is found
//   when
//   // a value is exact and the next one is also exact (determining the
//   direction
//   // of the sine wave).
//   size_t offset = 0;
//   while (true) {
//     float value = static_cast<float>(
//         std::sin(static_cast<float>(offset) / 2000.0f * 2 * M_PI));
//     float nextValue = static_cast<float>(
//         std::sin(static_cast<float>(offset + 1) / 2000.0f * 2 * M_PI));
//     if ((std::abs(data[0].second[0] - value) < requiredPrecision) &&
//         (std::abs(data[1].second[0] - nextValue) < requiredPrecision)) {
//       break;
//     }
//     if (offset > 2000) {
//       // But we know for sure it will never be that far in (as the wave has
//       // looped)
//       FAIL() << "Could not find the offset in the data";
//     }
//     offset++;
//   }
//   for (size_t i = 0; i < data.size(); i++) {
//     for (size_t j = 0; j < data[i].second.size(); j++) {
//       float value =
//           static_cast<float>(std::sin((i + offset) / 2000.0 * 2 * M_PI));
//       ASSERT_NEAR(data[i].second[j], value, requiredPrecision);
//     }
//   }
// }
