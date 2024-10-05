#include "Devices/Generic/AsyncDevice.h"

#include "Utils/Logger.h"
#include <regex>
#include <thread>

using namespace STIMWALKER_NAMESPACE::devices;

AsyncDevice::AsyncDevice(const std::chrono::milliseconds &keepAliveInterval)
    : m_KeepDeviceWorkerAliveInterval(keepAliveInterval), Device() {}

AsyncDevice::AsyncDevice(const std::chrono::microseconds &keepAliveInterval)
    : m_KeepDeviceWorkerAliveInterval(keepAliveInterval), Device() {}

void AsyncDevice::handleConnect() {
  bool isConnectionHandled = false;
  bool hasFailed = false;
  m_AsyncDeviceWorker = std::thread([this, &isConnectionHandled, &hasFailed]() {
    try {
      handleAsyncConnect();
    } catch (std::exception &) {
      hasFailed = true;
      return;
    }
    startKeepDeviceWorkerAlive();
    isConnectionHandled = true;
    m_AsyncDeviceContext.run();
  });

  // Wait until the connection is established
  while (!isConnectionHandled) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    if (hasFailed) {
      m_AsyncDeviceWorker.join();
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
  m_AsyncDeviceContext.stop();
  m_AsyncDeviceWorker.join();
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

void AsyncDevice::startKeepDeviceWorkerAlive() {
  // Add a keep-alive timer
  m_KeepDeviceWorkerAliveTimer =
      std::make_unique<asio::steady_timer>(m_AsyncDeviceContext);
  keepDeviceWorkerAlive(m_KeepDeviceWorkerAliveInterval);
}

void AsyncDevice::keepDeviceWorkerAlive(std::chrono::milliseconds timeout) {
  keepDeviceWorkerAlive(std::chrono::microseconds(timeout));
}

void AsyncDevice::keepDeviceWorkerAlive(std::chrono::microseconds timeout) {
  // Set a timer that will call [pingDeviceWorker] every [timeout] milliseconds
  m_KeepDeviceWorkerAliveTimer->expires_after(timeout);

  m_KeepDeviceWorkerAliveTimer->async_wait([this](const asio::error_code &ec) {
    // If ec is not false, it means the timer was stopped to change the
    // interval, or the device was disconnected. In both cases, do nothing and
    // return
    if (ec)
      return;

    // Otherwise, send a PING command to the device
    std::lock_guard<std::mutex> lock(m_AsyncDeviceMutex);
    pingDeviceWorker();

    // Once its done, repeat the process
    keepDeviceWorkerAlive(m_KeepDeviceWorkerAliveInterval);
  });
}

void AsyncDevice::pingDeviceWorker() {}

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
  m_AsyncDeviceContext.post(
      [this, &command, data = data, p = &promise, ignoreResponse]() mutable {
        std::lock_guard<std::mutex> lock(m_AsyncDeviceMutex);

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
