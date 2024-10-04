#include "Devices/Generic/AsyncDevice.h"

#include <regex>
#include <thread>

#include "Utils/Logger.h"

using namespace STIMWALKER_NAMESPACE::devices;

AsyncDevice::AsyncDevice() : m_KeepWorkerAliveInterval(1000), Device() {}

void AsyncDevice::handleConnect() {
  // Start a worker thread to run the device using the [_initialize] method
  // Start the worker thread
  bool isConnectionHandled = false;
  bool hasFailed = false;
  m_AsyncWorker = std::thread([this, &isConnectionHandled, &hasFailed]() {
    try {
      handleAsyncConnect();
    } catch (std::exception &) {
      hasFailed = true;
      return;
    }
    startKeepWorkerAlive();
    isConnectionHandled = true;
    m_AsyncContext.run();
  });

  // Wait until the connection is established
  while (!isConnectionHandled) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    if (hasFailed) {
      m_AsyncWorker.join();
      throw DeviceConnexionFailedException(
          "Error while connecting to the device");
    }
  }
}

void AsyncDevice::handleDisconnect() {
  // Just leave a bit of time if there are any pending commands to process
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  // Give the hand to inherited classes to clean up some stuff
  handleAsyncDisconnect();

  // Stop the worker thread
  m_AsyncContext.stop();
  m_AsyncWorker.join();
}

DeviceResponses AsyncDevice::sendFast(const DeviceCommands &command) {
  return sendInternalFast(command, nullptr);
}
DeviceResponses AsyncDevice::sendFast(const DeviceCommands &command,
                                      const char *data) {
  return sendInternalFast(command, std::string(data));
}
DeviceResponses AsyncDevice::sendFast(const DeviceCommands &command,
                                      const std::any &data) {
  return sendInternalFast(command, data);
}

void AsyncDevice::startKeepWorkerAlive() {
  // Add a keep-alive timer
  m_KeepWorkerAliveTimer = std::make_unique<asio::steady_timer>(m_AsyncContext);
  keepWorkerAlive(m_KeepWorkerAliveInterval);
}

void AsyncDevice::keepWorkerAlive(std::chrono::milliseconds timeout) {
  // Set a 5-second timer
  m_KeepWorkerAliveTimer->expires_after(timeout);

  m_KeepWorkerAliveTimer->async_wait([this](const asio::error_code &ec) {
    // If ec is not false, it means the timer was stopped to change the
    // interval, or the device was disconnected. In both cases, do nothing and
    // return
    if (ec)
      return;
    // Otherwise, send a PING command to the device
    std::lock_guard<std::mutex> lock(m_AsyncMutex);
    pingWorker();

    // Once its done, repeat the process
    keepWorkerAlive(m_KeepWorkerAliveInterval);
  });
}

void AsyncDevice::pingWorker() {}

DeviceResponses AsyncDevice::parseSendCommand(const DeviceCommands &command,
                                              const std::any &data) {
  return parseSendCommand(command, data, false);
}

DeviceResponses AsyncDevice::sendInternalFast(const DeviceCommands &command,
                                              const std::any &data) {
  auto &logger = utils::Logger::getInstance();

  if (!m_IsConnected) {
    logger.warning(
        "Cannot send a command to the device because it is not connected");
    return DeviceResponses::DEVICE_NOT_CONNECTED;
  }

  return parseSendCommand(command, data, true);
}

DeviceResponses AsyncDevice::parseSendCommand(const DeviceCommands &command,
                                              const std::any &data,
                                              bool ignoreResponse) {
  // Create a promise and get the future associated with it
  std::promise<DeviceResponses> promise;
  std::future<DeviceResponses> future = promise.get_future();
  // Send a command to the worker to relay commands to the device
  m_AsyncContext.post(
      [this, &command, data = data, p = &promise, ignoreResponse]() mutable {
        std::lock_guard<std::mutex> lock(m_AsyncMutex);

        // Parse the command and get the response
        auto response = parseAsyncSendCommand(command, data);

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
