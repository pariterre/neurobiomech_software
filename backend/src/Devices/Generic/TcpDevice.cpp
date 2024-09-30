#include "Devices/Generic/TcpDevice.h"

#if defined(_WIN32)
// #include <cfgmgr32.h>
// #include <setupapi.h>
// #include <windows.h>
#endif // _WIN32
#include <regex>
#include <thread>

#include "Utils/Logger.h"

using namespace STIMWALKER_NAMESPACE::devices;

TcpDevice::TcpDevice(const std::string &host, size_t port)
    : m_Host(host), m_Port(port) {}

TcpDevice::TcpDevice(const TcpDevice &other)
    : m_Host(other.m_Host), m_Port(other.m_Port) {
  // TODO Throws an exception if the serial port is open as it cannot be copied
}

std::vector<char> TcpDevice::read(size_t bufferSize) {
  size_t cursor = 0;
  std::vector<char> packets(bufferSize, 0);

  while (cursor < bufferSize) {
    try {
      int received = static_cast<int>(m_TcpSocket->receive(
          asio::buffer(packets.data() + cursor, bufferSize - cursor)));
      cursor += received;
    } catch (std::exception &) {
      disconnect();
      throw std::runtime_error("Error while reading the data disconnecting");
    }
  }

  return packets;
}

void TcpDevice::disconnect() {
  if (m_TcpSocket != nullptr && m_TcpSocket->is_open()) {
    m_TcpSocket->close();
  }

  AsyncDevice::disconnect();
}

void TcpDevice::handleConnect() {
  m_TcpSocket = std::make_unique<asio::ip::tcp::socket>(m_TcpContext);
  m_TcpSocket->connect(
      asio::ip::tcp::endpoint(asio::ip::address::from_string(m_Host),
                              static_cast<unsigned short>(m_Port)));
}
