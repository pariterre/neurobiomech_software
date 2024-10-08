#include <gtest/gtest.h>
#include <iostream>
#include <thread>

#include "Devices/MagstimRapidDevice.h"
#include "test_utils.h"

static double requiredPrecision(1e-10);

using namespace STIMWALKER_NAMESPACE;

TEST(Magstim, info) {
  auto magstim = devices::MagstimRapidDeviceMock::FindMagstimDevice();

  ASSERT_STREQ(magstim.getPort().c_str(), "MOCK");
  ASSERT_STREQ(magstim.getVid().c_str(), "067B");
  ASSERT_STREQ(magstim.getPid().c_str(), "2303");
}

TEST(Magstim, connect) {
  auto logger = TestLogger();
  auto magstim = devices::MagstimRapidDeviceMock::FindMagstimDevice();

  // Is not connected when created
  ASSERT_EQ(magstim.getIsConnected(), false);

  // Connect to the device, now shows as connected
  magstim.connect();
  ASSERT_EQ(magstim.getIsConnected(), true);
  ASSERT_TRUE(
      logger.contains("The device MagstimRapidDevice is now connected"));
  logger.clear();

  // Cannot connect twice
  magstim.connect();
  ASSERT_TRUE(logger.contains("Cannot connect to the device MagstimRapidDevice "
                              "because it is already connected"));
  logger.clear();

  // Disconnecting, shows as not connected anymore
  magstim.disconnect();
  ASSERT_EQ(magstim.getIsConnected(), false);
  ASSERT_TRUE(
      logger.contains("The device MagstimRapidDevice is now disconnected"));
  logger.clear();

  // Cannot disconnect twice
  magstim.disconnect();
  ASSERT_TRUE(logger.contains(
      "Cannot disconnect from the device MagstimRapidDevice because "
      "it is not connected"));
  logger.clear();
}

TEST(UsbDevice, Print) {
  auto logger = TestLogger();

  // Send a PRINT message to a USB device
  auto magstim = devices::MagstimRapidDeviceMock::FindMagstimDevice();
  magstim.send(devices::MagstimRapidCommands::PRINT, "Hello, world!");
  ASSERT_TRUE(
      logger.contains("Cannot send a command to the device "
                      "MagstimRapidDevice because it is not connected"));
  logger.clear();

  // Send a PRINT message to a USB device and wait for the response
  magstim.connect();
  magstim.send(devices::MagstimRapidCommands::PRINT, "Hello, world!");
  magstim.disconnect();

  // At least one message should have been sent
  ASSERT_TRUE(logger.contains("Hello, world!"));
  logger.clear();
}

TEST(Magstim, getTemperature) {
  // Connect the system and wait at least one POKE time and close
  auto magstim = devices::MagstimRapidDeviceMock::FindMagstimDevice();

  // Get the temperature
  magstim.connect();
  auto response = magstim.send(devices::MagstimRapidCommands::GET_TEMPERATURE);
  magstim.disconnect();
  ASSERT_EQ(response.getValue(), 42);
}

TEST(Magstim, setRapid) {
  auto logger = TestLogger();

  // Connect the system and set RTS to ON then to OFF
  auto magstim = devices::MagstimRapidDeviceMock::FindMagstimDevice();

  magstim.connect();
  magstim.send(devices::MagstimRapidCommands::SET_FAST_COMMUNICATION, true);
  ASSERT_TRUE(logger.contains("ON"));
  logger.clear();

  magstim.send(devices::MagstimRapidCommands::SET_FAST_COMMUNICATION, false);
  ASSERT_TRUE(logger.contains("OFF"));
  logger.clear();

  magstim.disconnect();
}

TEST(Magstim, arming) {
  auto logger = TestLogger();

  // Connect the system and wait at least one POKE time and close
  auto magstim = devices::MagstimRapidDeviceMock::FindMagstimDevice();

  // Trying to ARM the system without connecting should not work
  magstim.send(devices::MagstimRapidCommands::ARM);

  ASSERT_EQ(magstim.getIsArmed(), false);
  ASSERT_TRUE(
      logger.contains("Cannot send a command to the device "
                      "MagstimRapidDevice because it is not connected"));
  logger.clear();

  // Connect the system then send the ARM command
  magstim.connect();
  auto response = magstim.send(devices::MagstimRapidCommands::ARM);
  ASSERT_EQ(response.getValue(), devices::DeviceResponses::OK);
  ASSERT_EQ(magstim.getIsArmed(), true);
  ASSERT_TRUE(
      logger.contains("Armed the system and changed poke interval to 500 ms"));
  logger.clear();

  // Should not be able to ARM the system after it is already armed
  response = magstim.send(devices::MagstimRapidCommands::ARM);
  ASSERT_EQ(response.getValue(), devices::DeviceResponses::NOK);
  ASSERT_EQ(magstim.getIsArmed(), true);
  ASSERT_TRUE(logger.contains("Error: The device is already armed"));
  logger.clear();

  // Disarm the system
  response = magstim.send(devices::MagstimRapidCommands::DISARM);
  ASSERT_EQ(response.getValue(), devices::DeviceResponses::OK);
  ASSERT_EQ(magstim.getIsArmed(), false);
  ASSERT_TRUE(logger.contains(
      "Disarmed the system and changed poke interval to 5000 ms"));
  logger.clear();

  // Should not be able to DISARM the system after it is already disarmed
  response = magstim.send(devices::MagstimRapidCommands::DISARM);
  ASSERT_EQ(response.getValue(), devices::DeviceResponses::NOK);
  ASSERT_EQ(magstim.getIsArmed(), false);
  ASSERT_TRUE(logger.contains("Error: The device is already disarmed"));
  logger.clear();

  magstim.disconnect();
}

#ifndef SKIP_LONG_TESTS
TEST(Magstim, automaticPokingDisarmed) {
  auto logger = TestLogger();

  // Connect the system and wait at least one POKE time and close
  auto magstim = devices::MagstimRapidDeviceMock::FindMagstimDevice();
  ASSERT_EQ(magstim.getDisarmedPokeInterval(), std::chrono::milliseconds(5000));
  ASSERT_EQ(magstim.getKeepDeviceWorkerAliveInterval(),
            std::chrono::milliseconds(5000));

  magstim.connect();
  logger.clear();
  ASSERT_EQ(magstim.getKeepDeviceWorkerAliveInterval(),
            std::chrono::milliseconds(5000));

  std::this_thread::sleep_for(std::chrono::milliseconds(4000));
  ASSERT_FALSE(logger.contains("POKE"));

  std::this_thread::sleep_for(std::chrono::milliseconds(1500));
  ASSERT_TRUE(logger.contains("POKE"));

  magstim.disconnect();
}
#endif

#ifndef SKIP_LONG_TESTS
TEST(Magstim, automaticPokingArmed) {
  auto logger = TestLogger();

  // Connect the system and wait at least one POKE time and close
  auto magstim = devices::MagstimRapidDeviceMock::FindMagstimDevice();
  ASSERT_EQ(magstim.getArmedPokeInterval(), std::chrono::milliseconds(500));

  magstim.connect();
  magstim.send(devices::MagstimRapidCommands::ARM);
  ASSERT_EQ(magstim.getKeepDeviceWorkerAliveInterval(),
            std::chrono::milliseconds(500));
  logger.clear();

  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  ASSERT_FALSE(logger.contains("POKE"));

  std::this_thread::sleep_for(std::chrono::milliseconds(2200));
  magstim.disconnect();

  // The poke should have be called at least five times
  ASSERT_GE(logger.count("POKE"), 4);
}
#endif

TEST(Magstim, computeCrc) {
  auto magstim = devices::MagstimRapidDeviceMock::FindMagstimDevice();
  ASSERT_EQ(magstim.computeCrcInterface("Hello, world!"), "v");
}