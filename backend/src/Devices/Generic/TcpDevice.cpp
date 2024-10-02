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
    : m_Host(host), m_Port(port), m_TcpContext(),
      m_TcpSocket(asio::ip::tcp::socket(m_TcpContext)), AsyncDevice() {}

std::vector<char> TcpDevice::read(size_t bufferSize) {
  try {
    std::vector<char> packets(bufferSize, 0);
    m_TcpSocket.receive(asio::buffer(packets.data(), packets.size()));
    return packets;
  } catch (std::exception &e) {
    disconnect();
    throw std::runtime_error("Error while reading the data, disconnecting. " +
                             std::string(e.what()));
  }
}

void TcpDevice::disconnect() {
  if (m_TcpSocket.is_open()) {
    m_TcpSocket.close();
  }

  AsyncDevice::disconnect();
}

void TcpDevice::handleConnect() {
  m_TcpSocket.connect(
      asio::ip::tcp::endpoint(asio::ip::address::from_string(
                                  m_Host == "localhost" ? "127.0.0.1" : m_Host),
                              static_cast<unsigned short>(m_Port)));
}
