#include "Devices/Generic/AsyncDevice.h"

#include "Devices/Exceptions.h"
#include "Utils/Logger.h"
#include <regex>
#include <thread>

using namespace STIMWALKER_NAMESPACE::devices;

AsyncDevice::AsyncDevice(const std::chrono::microseconds &keepAliveInterval)
    : m_KeepDeviceWorkerAliveInterval(keepAliveInterval), Device() {}

AsyncDevice::~AsyncDevice() { stopDeviceWorkers(); }

void AsyncDevice::connectAsync() {
  auto &logger = utils::Logger::getInstance();

  if (m_IsConnected) {
    logger.warning("Cannot connect to the device " + deviceName() +
                   " because it is already connected");
    return;
  }

  m_AsyncDeviceContext.restart();
  m_HasFailedToConnect = false;
  m_AsyncDeviceWorker = std::thread([this]() {
    auto &logger = utils::Logger::getInstance();
    m_HasFailedToConnect = !handleConnect();

    if (m_HasFailedToConnect) {
      m_IsConnected = false;
      logger.fatal("Could not connect to the device " + deviceName());
      return;
    }

    logger.info("The device " + deviceName() + " is now connected");
    startKeepDeviceWorkerAlive();
    m_IsConnected = true;
    m_AsyncDeviceContext.run();
  });
}

bool AsyncDevice::connect() {
  connectAsync();
  while (!m_IsConnected && !m_HasFailedToConnect) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  if (m_HasFailedToConnect) {
    stopDeviceWorkers();
    return false;
  }
  return true;
}

bool AsyncDevice::disconnect() {
  auto &logger = utils::Logger::getInstance();

  if (!m_IsConnected) {
    logger.warning("Cannot disconnect from the device " + deviceName() +
                   " because it is not connected");
    return true;
  }

  // Just leave a bit of time if there are any pending commands to process
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  // Give the hand to inherited classes to clean up some stuff
  m_IsConnected = !handleDisconnect();
  if (m_IsConnected) {
    logger.fatal("Could not disconnect from the device " + deviceName());
    return false;
  }

  stopDeviceWorkers();
  logger.info("The device " + deviceName() + " is now disconnected");
  return true;
}

void AsyncDevice::stopDeviceWorkers() {
  if (m_IsConnected) {
    disconnect();
  }

  // Stop the worker thread
  if (!m_AsyncDeviceContext.stopped()) {
    m_AsyncDeviceContext.stop();
  }

  if (m_AsyncDeviceWorker.joinable()) {
    m_AsyncDeviceWorker.join();
  }
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
