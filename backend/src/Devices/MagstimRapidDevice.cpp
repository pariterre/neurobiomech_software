#include "Devices/MagstimRapidDevice.h"

#if defined(_WIN32)
#include <cfgmgr32.h>
#include <setupapi.h>
#include <windows.h>
#else // Linux or macOS
#include <filesystem>
#include <fstream>
#endif // _WIN32
#include <regex>

#include "Utils/Logger.h"

using namespace STIMWALKER_NAMESPACE;
using namespace STIMWALKER_NAMESPACE::devices;

MagstimRapidDevice MagstimRapidDevice::FindMagstimDevice() {
  return MagstimRapidDevice(UsbDevice::fromVidAndPid("067B", "2303").getPort());
}

MagstimRapidDevice::MagstimRapidDevice(const std::string &port)
    : m_IsArmed(false), m_ArmedPokeInterval(std::chrono::milliseconds(500)),
      m_DisarmedPokeInterval(std::chrono::milliseconds(5000)),
      UsbDevice(port, "067B", "2303") {
  m_KeepWorkerAliveInterval = m_DisarmedPokeInterval;
}

DeviceResponses
MagstimRapidDevice::parseAsyncSendCommand(const DeviceCommands &command,
                                          const std::any &data) {
  auto &logger = utils::Logger::getInstance();
  std::string commandString;
  std::string response;

  try {
    switch (command.getValue()) {
    case MagstimRapidCommands::PRINT:
      logger.info("Sent command: " + std::any_cast<std::string>(data));

      return DeviceResponses::OK;

    case MagstimRapidCommands::POKE:
      logger.info("Sent command: " + std::any_cast<std::string>(data));
      break;

    case MagstimRapidCommands::GET_TEMPERATURE:
      // We do not need to check if the system is armed for this command
      commandString = "F@";
      // asio::write(*m_SerialPort,
      //             asio::buffer(commandString + computeCRC(commandString)));

      // Wait for the response (9 bytes)
      // response = std::string(9, '\0');

      // asio::read(*m_SerialPort, asio::buffer(response));
      return 42;

    case MagstimRapidCommands::SET_FAST_COMMUNICATION:
      setFastCommunication(std::any_cast<bool>(data));
      return DeviceResponses::OK;

    case MagstimRapidCommands::ARM:
      if (m_IsArmed) {
        throw MagsimRapidAlreadyArmedException("The device is already armed");
      }
      m_IsArmed = command.getValue() == MagstimRapidCommands::ARM;

      changePokeInterval(std::chrono::milliseconds(
          m_IsArmed ? m_ArmedPokeInterval : m_DisarmedPokeInterval));

      logger.info(std::string(m_IsArmed ? "Armed" : "Disarmed") +
                  " the system and changed poke interval to " +
                  std::to_string(m_KeepWorkerAliveInterval.count()) + " ms");
      return DeviceResponses::OK;

    case MagstimRapidCommands::DISARM:
      if (!m_IsArmed) {
        throw MagsimRapidNotArmedException("The device is already disarmed");
      }

      m_IsArmed = command.getValue() == MagstimRapidCommands::ARM;

      changePokeInterval(std::chrono::milliseconds(
          m_IsArmed ? m_ArmedPokeInterval : m_DisarmedPokeInterval));

      logger.info(std::string(m_IsArmed ? "Armed" : "Disarmed") +
                  " the system and changed poke interval to " +
                  std::to_string(m_KeepWorkerAliveInterval.count()) + " ms");
      return DeviceResponses::OK;
    }
  } catch (const std::bad_any_cast &) {
    logger.fatal("The data you provided with the command (" +
                 command.toString() + ") is invalid");
    return DeviceResponses::NOK;
  } catch (const std::exception &e) {
    logger.fatal("Error: " + std::string(e.what()));
    return DeviceResponses::NOK;
  }

  return DeviceResponses::COMMAND_NOT_FOUND;
}

void MagstimRapidDevice::pingWorker() {
  parseAsyncSendCommand(MagstimRapidCommands::POKE, std::string("POKE"));
}

std::string MagstimRapidDevice::computeCrc(const std::string &data) {
  // Convert the command string to sum of ASCII/byte values
  int commandSum = 0;
  for (const auto &c : data) {
    commandSum += c;
  }

  // Convert command sum to binary, then invert and return 8-bit character value
  return std::string(1, static_cast<char>(~commandSum & 0xff));
}

void MagstimRapidDevice::changePokeInterval(
    std::chrono::milliseconds interval) {
  // Stop the timer

  // Compute the remaining time before the next PING command was supposed to be
  // sent
  auto remainingTime = std::chrono::duration_cast<std::chrono::milliseconds>(
      m_KeepWorkerAliveTimer->expiry() - asio::steady_timer::clock_type::now());
  auto elapsedTime = m_KeepWorkerAliveInterval - remainingTime;

  // Set the interval to the requested value
  m_KeepWorkerAliveInterval = interval;

  // Send a keep alive command with the remaining time
  m_KeepWorkerAliveTimer->cancel();
  keepWorkerAlive(interval - elapsedTime);
}

// --- MOCKER SECTION --- //
MagstimRapidDeviceMock::MagstimRapidDeviceMock(const std::string &port)
    : MagstimRapidDevice(port) {}

MagstimRapidDeviceMock MagstimRapidDeviceMock::FindMagstimDevice() {
  return MagstimRapidDeviceMock("MOCK");
}

std::string
MagstimRapidDeviceMock::computeCrcInterface(const std::string &data) {
  return computeCrc(data);
}

void MagstimRapidDeviceMock::setFastCommunication(bool isFast) {
  auto &logger = utils::Logger::getInstance();

  if (isFast) {
    logger.info("RTS set to ON");
  } else {
    logger.info("RTS set to OFF");
  }
}

void MagstimRapidDeviceMock::handleAsyncConnect() {}

void MagstimRapidDeviceMock::handleAsyncDisconnect() {}