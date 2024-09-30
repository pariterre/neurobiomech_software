#include <gtest/gtest.h>
#include <iostream>

#include "Devices/MagstimRapidDevice.h"
#include "Utils/Logger.h"

static double requiredPrecision(1e-10);

using namespace STIMWALKER_NAMESPACE;

TEST(Magstim, connect) {
  auto magstim = devices::MagstimRapidDeviceMock::FindMagstimDevice();

  // Is not connected when created
  ASSERT_EQ(magstim.getIsConnected(), false);

  // Connect to the device, now shows as connected
  magstim.connect();
  ASSERT_EQ(magstim.getIsConnected(), true);

  // Cannot connect twice
  EXPECT_THROW(magstim.connect(), devices::DeviceIsConnectedException);

  // Disconnecting, shows as not connected anymore
  magstim.disconnect();
  ASSERT_EQ(magstim.getIsConnected(), false);

  // Cannot disconnect twice
  EXPECT_THROW(magstim.disconnect(), devices::DeviceIsNotConnectedException);
}
// TEST(UsbDevice, Print) {
//   auto &logger = utils::Logger::getInstance();
//   logger.setLogLevel(utils::Logger::INFO);

//   std::vector<std::string> messagesToDevice;
//   logger.onNewLog.listen([&messagesToDevice](const std::string &message) {
//     messagesToDevice.push_back(message);
//   });

//   // Send a PRINT message to a USB device
//   auto magstim = devices::MagstimRapidDeviceMock::FindMagstimDevice();
//   magstim.send(devices::MagstimRapidCommands::PRINT, "Hello, world!", true);

//   // At least one message should have been sent
//   ASSERT_GE(messagesToDevice.size(), 1);
//   bool hasPrint = false;
//   for (const auto &message : messagesToDevice) {
//     if (message.find("Hello, world!") != std::string::npos) {
//       hasPrint = true;
//     }
//   }
//   ASSERT_TRUE(hasPrint);
// }

// TEST(Magstim, getTemperature) {
//   auto &logger = utils::Logger::getInstance();
//   logger.setLogLevel(utils::Logger::INFO);

//   std::vector<std::string> messagesToDevice;
//   logger.onNewLog.listen([&messagesToDevice](const std::string &message) {
//     messagesToDevice.push_back(message);
//   });

//   // Connect the system and wait at least one POKE time and close
//   auto magstim = devices::MagstimRapidDeviceMock::FindMagstimDevice();
//   magstim.connect();

//   // Get the temperature
//   auto response =
//   magstim.send(devices::MagstimRapidCommands::GET_TEMPERATURE);
//   ASSERT_EQ(response, 42);

//   // At least one message should have been sent
//   ASSERT_GE(messagesToDevice.size(), 1);
//   bool hasTemperature = false;
//   for (const auto &message : messagesToDevice) {
//     if (message.find("Temperature") != std::string::npos) {
//       hasTemperature = true;
//     }
//   }
//   ASSERT_TRUE(hasTemperature);
// }

// TEST(Magstim, setRapid) {
//   auto &logger = utils::Logger::getInstance();
//   logger.setLogLevel(utils::Logger::INFO);

//   std::vector<std::string> messagesToDevice;
//   logger.onNewLog.listen([&messagesToDevice](const std::string &message) {
//     messagesToDevice.push_back(message);
//   });

//   // Connect the system and set RTS to ON then to OFF
//   auto magstim = devices::MagstimRapidDeviceMock::FindMagstimDevice();
//   magstim.send(devices::MagstimRapidCommands::SET_FAST_COMMUNICATION, true);
//   magstim.send(devices::MagstimRapidCommands::SET_FAST_COMMUNICATION, false);

//   // We should have at least 2 messages, one for the ON and one for the OFF
//   ASSERT_GE(messagesToDevice.size(), 2);
//   bool hasSetOn = false;
//   bool hasSetOff = false;
//   for (const auto &message : messagesToDevice) {
//     if (message.find("ON") != std::string::npos) {
//       hasSetOn = true;
//     }
//     if (message.find("OFF") != std::string::npos) {
//       hasSetOff = true;
//     }
//   }
//   ASSERT_TRUE(hasSetOn);
//   ASSERT_TRUE(hasSetOff);
// }

// TEST(Magstim, arming) {
//   auto &logger = utils::Logger::getInstance();
//   logger.setLogLevel(utils::Logger::INFO);

//   std::vector<std::string> messagesToDevice;
//   logger.onNewLog.listen([&messagesToDevice](const std::string &message) {
//     messagesToDevice.push_back(message);
//   });

//   // Connect the system and wait at least one POKE time and close
//   auto magstim = devices::MagstimRapidDeviceMock::FindMagstimDevice();

//   // Trying to ARM the system without connecting should not work
//   auto response = magstim.send(devices::MagstimRapidCommands::ARM);
//   ASSERT_EQ(response, devices::DeviceResponses::NOK);

//   // Connect the system then send the ARM command
//   magstim.connect();
//   response = magstim.send(devices::MagstimRapidCommands::ARM);
//   ASSERT_EQ(response, devices::DeviceResponses::OK);
//   ASSERT_EQ(magstim.getIsArmed(), true);

//   response = magstim.send(devices::MagstimRapidCommands::DISARM);
//   ASSERT_EQ(response, devices::DeviceResponses::OK);
//   ASSERT_EQ(magstim.getIsArmed(), false);
//   magstim.disconnect();

//   // Should not be able to ARM the system after it is disconnected
//   response = magstim.send(devices::MagstimRapidCommands::DISARM);
//   ASSERT_EQ(response, devices::DeviceResponses::OK);

//   // The callbacks should have been called at least 2 times, one for the ARM
//   and
//   // one for the DISARM
//   ASSERT_GE(messagesToDevice.size(), 2);
//   bool hasArmed = false;
//   bool hasDisarmed = false;
//   for (const auto &message : messagesToDevice) {
//     if (message.find("ARM") != std::string::npos) {
//       hasArmed = true;
//     }
//     if (message.find("DISARM") != std::string::npos) {
//       hasDisarmed = true;
//     }
//   }
//   ASSERT_TRUE(hasArmed);
//   ASSERT_TRUE(hasDisarmed);
// }

// TEST(Magstim, automaticPokingNotArmed) {
//   auto &logger = utils::Logger::getInstance();
//   std::vector<std::string> messagesToDevice;
//   logger.onNewLog.listen([&messagesToDevice](const std::string &message) {
//     messagesToDevice.push_back(message);
//   });

//   // Connect the system and wait at least one POKE time and close
//   auto magstim = devices::MagstimRapidDeviceMock::FindMagstimDevice();
//   magstim.connect();
//   std::this_thread::sleep_for(std::chrono::milliseconds(6000));
//   magstim.disconnect();

//   // The messagesToDevices should have be called at least once but maximum 2
//   ASSERT_GE(messagesToDevice.size(), 1);
//   ASSERT_LE(messagesToDevice.size(), 2);

//   // The first message should be a POKE command, but the message contains the
//   // time, so it must be parsed for the POKE tag
//   ASSERT_TRUE(messagesToDevice[0].find("POKE") != std::string::npos);
// }

// TEST(Magstim, automaticPokingDisarmed) {
//   auto &logger = utils::Logger::getInstance();
//   std::vector<std::string> messagesToDevice;
//   logger.onNewLog.listen([&messagesToDevice](const std::string &message) {
//     messagesToDevice.push_back(message);
//   });

//   // Connect the system and wait at least one POKE time and close
//   auto magstim = devices::MagstimRapidDeviceMock::FindMagstimDevice();

//   // Connect the system without sending the ARM command
//   ASSERT_EQ(magstim.getDisarmedPokeInterval(),
//   std::chrono::milliseconds(5000)); magstim.connect();
//   std::this_thread::sleep_for(std::chrono::milliseconds(6000));
//   magstim.disconnect();

//   // The messagesToDevices should have be called at least once
//   size_t pokeCount = 0;
//   for (const auto &message : messagesToDevice) {
//     if (message.find("POKE") != std::string::npos) {
//       pokeCount++;
//     }
//   }
//   ASSERT_GE(pokeCount, 1);
// }

// TEST(Magstim, automaticPokingArmed) {
//   auto &logger = utils::Logger::getInstance();
//   std::vector<std::string> messagesToDevice;
//   logger.onNewLog.listen([&messagesToDevice](const std::string &message) {
//     messagesToDevice.push_back(message);
//   });

//   // Connect the system and wait at least one POKE time and close
//   auto magstim = devices::MagstimRapidDeviceMock::FindMagstimDevice();

//   // Connect the system without sending the ARM command
//   ASSERT_EQ(magstim.getArmedPokeInterval(), std::chrono::milliseconds(500));
//   magstim.connect();
//   magstim.send(devices::MagstimRapidCommands::ARM);
//   std::this_thread::sleep_for(std::chrono::milliseconds(6000));
//   magstim.disconnect();

//   // The messagesToDevices should have be called at least five times
//   size_t pokeCount = 0;
//   for (const auto &message : messagesToDevice) {
//     if (message.find("POKE") != std::string::npos) {
//       pokeCount++;
//     }
//   }
//   ASSERT_GE(pokeCount, 5);
// }