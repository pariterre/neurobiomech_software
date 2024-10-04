#include "Devices/Generic/Device.h"

#include <regex>
#include <thread>

#include "Utils/Logger.h"

using namespace STIMWALKER_NAMESPACE::devices;

Device::~Device() {
  if (m_IsConnected) {
    disconnect();
  }
}

void Device::connect() {
  auto &logger = utils::Logger::getInstance();

  if (m_IsConnected) {
    logger.warning(
        "Cannot connect to the device because it is already connected");
    return;
  }

  handleConnect();

  m_IsConnected = true;
  logger.info("The device is now connected");
}

void Device::disconnect() {
  auto &logger = utils::Logger::getInstance();

  if (!m_IsConnected) {
    logger.warning(
        "Cannot disconnect from the device because it is not connected");
    return;
  }

  handleDisconnect();

  m_IsConnected = false;
  logger.info("The device is now disconnected");
}

DeviceResponses Device::send(const DeviceCommands &command) {
  return sendInternal(command, nullptr);
}
DeviceResponses Device::send(const DeviceCommands &command, const char *data) {
  return sendInternal(command, std::string(data));
}
DeviceResponses Device::send(const DeviceCommands &command,
                             const std::any &data) {
  return sendInternal(command, data);
}

DeviceResponses Device::sendInternal(const DeviceCommands &command,
                                     const std::any &data) {
  auto &logger = utils::Logger::getInstance();

  if (!m_IsConnected) {
    logger.warning(
        "Cannot send a command to the device because it is not connected");
    return DeviceResponses::DEVICE_NOT_CONNECTED;
  }

  return parseSendCommand(command, data);
}
