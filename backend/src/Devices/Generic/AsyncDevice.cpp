#include "Devices/Generic/AsyncDevice.h"

#if defined(_WIN32)
// #include <cfgmgr32.h>
// #include <setupapi.h>
// #include <windows.h>
#endif // _WIN32
#include <regex>
#include <thread>

#include "Utils/Logger.h"

using namespace STIMWALKER_NAMESPACE::devices;

AsyncDevice::~AsyncDevice() {
  if (m_IsConnected) {
    disconnect();
  }
}

void AsyncDevice::connect() {
  auto &logger = utils::Logger::getInstance();

  if (m_IsConnected) {
    logger.warning(
        "Cannot connect to the device because it is already connected");
    return;
  }

  // Start a worker thread to run the device using the [_initialize] method
  // Start the worker thread
  bool isConnectionHandled = false;
  bool hasFailed = false;
  m_AsyncWorker = std::thread([this, &isConnectionHandled, &hasFailed]() {
    try {
      handleConnect();
    } catch (std::exception &) {
      hasFailed = true;
      return;
    }
    isConnectionHandled = true;
    m_AsyncContext.run();
    utils::Logger::getInstance().info("Worker thread has stopped");
  });

  // Give a bit of time for the worker thread to start
  while (!isConnectionHandled) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    if (hasFailed) {
      m_AsyncWorker.join();
      throw DeviceConnexionFailedException(
          "Error while connecting to the device");
    }
  }

  m_IsConnected = true;
  logger.info("The device is now connected");
}

void AsyncDevice::disconnect() {
  auto &logger = utils::Logger::getInstance();

  if (!m_IsConnected) {
    logger.warning(
        "Cannot disconnect from the device because it is not connected");
    return;
  }

  // Just leave a bit of time if there are any pending commands to process
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  // Stop the worker thread
  m_AsyncContext.stop();
  m_AsyncWorker.join();

  m_IsConnected = false;
  logger.info("The device is now disconnected");
}

DeviceResponses AsyncDevice::send(const DeviceCommands &command) {
  return sendInternal(command, nullptr, false);
}
DeviceResponses AsyncDevice::send(const DeviceCommands &command,
                                  const char *data) {
  return sendInternal(command, std::string(data), false);
}
DeviceResponses AsyncDevice::send(const DeviceCommands &command,
                                  const std::any &data) {
  return sendInternal(command, data, false);
}

DeviceResponses AsyncDevice::sendFast(const DeviceCommands &command) {
  return sendInternal(command, nullptr, true);
}
DeviceResponses AsyncDevice::sendFast(const DeviceCommands &command,
                                      const char *data) {
  return sendInternal(command, std::string(data), true);
}
DeviceResponses AsyncDevice::sendFast(const DeviceCommands &command,
                                      const std::any &data) {
  return sendInternal(command, data, true);
}

DeviceResponses AsyncDevice::sendInternal(const DeviceCommands &command,
                                          const std::any &data,
                                          bool ignoreResponse) {
  auto &logger = utils::Logger::getInstance();

  if (!m_IsConnected) {
    logger.warning(
        "Cannot send a command to the device because it is not connected");
    return DeviceResponses::DEVICE_NOT_CONNECTED;
  }

  // Create a promise and get the future associated with it
  std::promise<DeviceResponses> promise;
  std::future<DeviceResponses> future = promise.get_future();
  // Send a command to the worker to relay commands to the device
  m_AsyncContext.post(
      [this, command, data = data, p = &promise, ignoreResponse]() mutable {
        std::lock_guard<std::mutex> lock(m_AsyncMutex);

        // Parse the command and get the response
        auto response = parseCommand(command, data);

        // Set the response value in the promise
        if (!ignoreResponse) {
          p->set_value(response);
        }
      });

  if (ignoreResponse) {
    return DeviceResponses::OK;
  }

  // Wait for the response from the worker thread and return the result
  return future.get();
}

DeviceResponses AsyncDevice::parseCommand(const DeviceCommands &command,
                                          const std::any &data) {
  // TODO Add a flusher for the serial port

  return DeviceResponses::COMMAND_NOT_FOUND;
}
