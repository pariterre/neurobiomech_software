#include "Devices/Generic/TcpDevice.h"

#if defined(_WIN32)
// #include <cfgmgr32.h>
// #include <setupapi.h>
// #include <windows.h>
#endif // _WIN32
#include <regex>
#include <thread>

#include "Utils/Logger.h"

using namespace NEUROBIO_NAMESPACE::devices;

TcpDevice::TcpDevice(const std::string &host, size_t port,
                     const std::chrono::microseconds &keepAliveInterval)
    : m_Host(host), m_Port(port), m_TcpContext(),
      m_TcpSocket(asio::ip::tcp::socket(m_TcpContext)),
      AsyncDevice(keepAliveInterval) {}

std::vector<char> TcpDevice::read(size_t bufferSize) {
  std::vector<char> buffer(bufferSize);
  read(buffer);
  return buffer;
}

bool TcpDevice::read(std::vector<char> &buffer) {
  try {
    m_TcpSocket.receive(asio::buffer(buffer.data(), buffer.size()));
    return true;
  } catch (std::exception &e) {
    utils::Logger::getInstance().fatal(
        "Error while reading the data to the device " + deviceName() +
        ", disconnecting. (" + std::string(e.what()) + ")");
    disconnect();
    return false;
  }
}

bool TcpDevice::write(const std::string &data) {
  try {
    m_TcpSocket.send(asio::buffer(data));
    return true;
  } catch (std::exception &e) {
    utils::Logger::getInstance().fatal(
        "Error while writing the data to the device " + deviceName() +
        ", disconnecting. (" + std::string(e.what()) + ")");
    disconnect();
    return false;
  }
}

bool TcpDevice::handleConnect() {
  try {
    m_TcpSocket.connect(asio::ip::tcp::endpoint(
        asio::ip::address::from_string(m_Host == "localhost" ? "127.0.0.1"
                                                             : m_Host),
        static_cast<unsigned short>(m_Port)));
  } catch (std::exception &) {
    return false;
  }

  return true;
}

bool TcpDevice::handleDisconnect() {
  if (m_TcpSocket.is_open()) {
    m_TcpSocket.close();
  }

  return true;
}