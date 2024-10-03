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
  std::vector<char> buffer(bufferSize);
  read(buffer);
  return buffer;
}

void TcpDevice::read(std::vector<char> &buffer) {
  try {
    m_TcpSocket.receive(asio::buffer(buffer.data(), buffer.size()));
  } catch (std::exception &e) {
    disconnect();
    throw std::runtime_error("Error while reading the data, disconnecting. " +
                             std::string(e.what()));
  }
}

void TcpDevice::write(const std::string &data) {
  try {
    m_TcpSocket.send(asio::buffer(data));
  } catch (std::exception &e) {
    disconnect();
    throw std::runtime_error("Error while writing the data, disconnecting. " +
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
