#include <gtest/gtest.h>
#include <iostream>
#include <thread>

#include "Devices/MagstimRapidDevice.h"
#include "Utils/Logger.h"

static double requiredPrecision(1e-10);

using namespace STIMWALKER_NAMESPACE;

auto &logger = utils::Logger::getInstance();
size_t listenToLogger(std::vector<std::string> &messagesToDevice) {
  logger.setShouldPrintToConsole(false);

  size_t loggerId =
      logger.onNewLog.listen([&messagesToDevice](const std::string &message) {
        messagesToDevice.push_back(message);
      });
  return loggerId;
}

bool findMessageInLogger(const std::vector<std::string> &messagesToDevice,
                         const std::string &message) {
  for (const auto &msg : messagesToDevice) {
    if (msg.find(message) != std::string::npos) {
      return true;
    }
  }
  return false;
}

void clearListenToLogger(size_t loggerId) { logger.onNewLog.clear(loggerId); }

TEST(Magstim, info) {
  auto magstim = devices::MagstimRapidDeviceMock::FindMagstimDevice();

  ASSERT_STREQ(magstim.getPort().c_str(), "MOCK");
  ASSERT_STREQ(magstim.getVid().c_str(), "067B");
  ASSERT_STREQ(magstim.getPid().c_str(), "2303");
}

TEST(Magstim, connect) {
  std::vector<std::string> messagesToDevice;
  size_t loggerId = listenToLogger(messagesToDevice);

  auto magstim = devices::MagstimRapidDeviceMock::FindMagstimDevice();

  // Is not connected when created
  ASSERT_EQ(magstim.getIsConnected(), false);

  // Connect to the device, now shows as connected
  magstim.connect();
  ASSERT_EQ(magstim.getIsConnected(), true);
  ASSERT_TRUE(findMessageInLogger(
      messagesToDevice, "The device MagstimRapidDevice is now connected"));
  messagesToDevice.clear();

  // Cannot connect twice
  magstim.connect();
  ASSERT_TRUE(findMessageInLogger(
      messagesToDevice, "Cannot connect to the device MagstimRapidDevice "
                        "because it is already connected"));
  messagesToDevice.clear();

  // Disconnecting, shows as not connected anymore
  magstim.disconnect();
  ASSERT_EQ(magstim.getIsConnected(), false);
  ASSERT_TRUE(findMessageInLogger(
      messagesToDevice, "The device MagstimRapidDevice is now disconnected"));
  messagesToDevice.clear();

  // Cannot disconnect twice
  magstim.disconnect();
  ASSERT_TRUE(findMessageInLogger(
      messagesToDevice,
      "Cannot disconnect from the device MagstimRapidDevice because "
      "it is not connected"));

  clearListenToLogger(loggerId);
}

TEST(UsbDevice, Print) {
  std::vector<std::string> messagesToDevice;
  size_t loggerId = listenToLogger(messagesToDevice);

  // Send a PRINT message to a USB device
  auto magstim = devices::MagstimRapidDeviceMock::FindMagstimDevice();
  magstim.send(devices::MagstimRapidCommands::PRINT, "Hello, world!");
  ASSERT_TRUE(findMessageInLogger(
      messagesToDevice, "Cannot send a command to the device "
                        "MagstimRapidDevice because it is not connected"));
  messagesToDevice.clear();

  // Send a PRINT message to a USB device and wait for the response
  magstim.connect();
  magstim.send(devices::MagstimRapidCommands::PRINT, "Hello, world!");
  magstim.disconnect();
  clearListenToLogger(loggerId);

  // At least one message should have been sent
  ASSERT_TRUE(findMessageInLogger(messagesToDevice, "Hello, world!"));

  clearListenToLogger(loggerId);
}

TEST(Magstim, getTemperature) {
  std::vector<std::string> messagesToDevice;
  size_t loggerId = listenToLogger(messagesToDevice);

  // Connect the system and wait at least one POKE time and close
  auto magstim = devices::MagstimRapidDeviceMock::FindMagstimDevice();

  // Get the temperature
  magstim.connect();
  auto response = magstim.send(devices::MagstimRapidCommands::GET_TEMPERATURE);
  magstim.disconnect();
  clearListenToLogger(loggerId);
  ASSERT_EQ(response.getValue(), 42);

  clearListenToLogger(loggerId);
}

// TEST(Magstim, setRapid) {
//   std::vector<std::string> messagesToDevice;
//   size_t loggerId = listenToLogger(messagesToDevice);

//   // Connect the system and set RTS to ON then to OFF
//   auto magstim = devices::MagstimRapidDeviceMock::FindMagstimDevice();

//   magstim.connect();
//   magstim.send(devices::MagstimRapidCommands::SET_FAST_COMMUNICATION, true);
//   ASSERT_TRUE(findMessageInLogger(messagesToDevice, "ON"));
//   messagesToDevice.clear();

//   magstim.send(devices::MagstimRapidCommands::SET_FAST_COMMUNICATION, false);
//   ASSERT_TRUE(findMessageInLogger(messagesToDevice, "OFF"));
//   messagesToDevice.clear();

//   magstim.disconnect();
//   clearListenToLogger(loggerId);
// }

TEST(Magstim, arming) {
  std::vector<std::string> messagesToDevice;
  size_t loggerId = listenToLogger(messagesToDevice);

  // Connect the system and wait at least one POKE time and close
  auto magstim = devices::MagstimRapidDeviceMock::FindMagstimDevice();

  // Trying to ARM the system without connecting should not work
  magstim.send(devices::MagstimRapidCommands::ARM);

  ASSERT_EQ(magstim.getIsArmed(), false);
  ASSERT_TRUE(findMessageInLogger(
      messagesToDevice, "Cannot send a command to the device "
                        "MagstimRapidDevice because it is not connected"));
  messagesToDevice.clear();

  // Connect the system then send the ARM command
  magstim.connect();
  auto response = magstim.send(devices::MagstimRapidCommands::ARM);
  ASSERT_EQ(response.getValue(), devices::DeviceResponses::OK);
  ASSERT_EQ(magstim.getIsArmed(), true);
  ASSERT_TRUE(findMessageInLogger(
      messagesToDevice,
      "Armed the system and changed poke interval to 500 ms"));
  messagesToDevice.clear();

  // Should not be able to ARM the system after it is already armed
  response = magstim.send(devices::MagstimRapidCommands::ARM);
  ASSERT_EQ(response.getValue(), devices::DeviceResponses::NOK);
  ASSERT_EQ(magstim.getIsArmed(), true);
  ASSERT_TRUE(findMessageInLogger(messagesToDevice,
                                  "Error: The device is already armed"));
  messagesToDevice.clear();

  // Disarm the system
  response = magstim.send(devices::MagstimRapidCommands::DISARM);
  ASSERT_EQ(response.getValue(), devices::DeviceResponses::OK);
  ASSERT_EQ(magstim.getIsArmed(), false);
  ASSERT_TRUE(findMessageInLogger(
      messagesToDevice,
      "Disarmed the system and changed poke interval to 5000 ms"));
  messagesToDevice.clear();

  // Should not be able to DISARM the system after it is already disarmed
  response = magstim.send(devices::MagstimRapidCommands::DISARM);
  ASSERT_EQ(response.getValue(), devices::DeviceResponses::NOK);
  ASSERT_EQ(magstim.getIsArmed(), false);
  ASSERT_TRUE(findMessageInLogger(messagesToDevice,
                                  "Error: The device is already disarmed"));
  messagesToDevice.clear();

  magstim.disconnect();
  clearListenToLogger(loggerId);
}

#ifndef SKIP_LONG_TESTS
TEST(Magstim, automaticPokingDisarmed) {
  std::vector<std::string> messagesToDevice;
  size_t loggerId = listenToLogger(messagesToDevice);

  // Connect the system and wait at least one POKE time and close
  auto magstim = devices::MagstimRapidDeviceMock::FindMagstimDevice();
  ASSERT_EQ(magstim.getDisarmedPokeInterval(), std::chrono::milliseconds(5000));
  ASSERT_EQ(magstim.getKeepDeviceWorkerAliveInterval(),
            std::chrono::milliseconds(5000));

  magstim.connect();
  messagesToDevice.clear();
  ASSERT_EQ(magstim.getKeepDeviceWorkerAliveInterval(),
            std::chrono::milliseconds(5000));

  std::this_thread::sleep_for(std::chrono::milliseconds(4000));
  ASSERT_EQ(messagesToDevice.size(), 0);

  std::this_thread::sleep_for(std::chrono::milliseconds(1500));
  ASSERT_EQ(messagesToDevice.size(), 1);
  ASSERT_TRUE(findMessageInLogger(messagesToDevice, "POKE"));

  magstim.disconnect();
  clearListenToLogger(loggerId);
}
#endif

#ifndef SKIP_LONG_TESTS
TEST(Magstim, automaticPokingArmed) {
  std::vector<std::string> messagesToDevice;
  size_t loggerId = listenToLogger(messagesToDevice);

  // Connect the system and wait at least one POKE time and close
  auto magstim = devices::MagstimRapidDeviceMock::FindMagstimDevice();
  ASSERT_EQ(magstim.getArmedPokeInterval(), std::chrono::milliseconds(500));

  magstim.connect();
  magstim.send(devices::MagstimRapidCommands::ARM);
  ASSERT_EQ(magstim.getKeepDeviceWorkerAliveInterval(),
            std::chrono::milliseconds(500));
  messagesToDevice.clear();

  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  ASSERT_EQ(messagesToDevice.size(), 0);

  std::this_thread::sleep_for(std::chrono::milliseconds(2200));
  ASSERT_GE(messagesToDevice.size(), 4); // 4 pokes in 2 seconds
  magstim.disconnect();
  clearListenToLogger(loggerId);

  // The messagesToDevices should have be called at least five times
  size_t pokeCount = 0;
  for (const auto &message : messagesToDevice) {
    if (message.find("POKE") != std::string::npos) {
      pokeCount++;
    }
  }
  ASSERT_GE(pokeCount, 4);
}
#endif

TEST(Magstim, computeCrc) {
  auto magstim = devices::MagstimRapidDeviceMock::FindMagstimDevice();
  ASSERT_EQ(magstim.computeCrcInterface("Hello, world!"), "v");
}