#include "Devices/Generic/SerialPortDevice.h"

#if defined(_WIN32)
// #include <cfgmgr32.h>
// #include <setupapi.h>
// #include <windows.h>
#endif // _WIN32
#include <regex>
#include <thread>

#include "Utils/Logger.h"

using namespace STIMWALKER_NAMESPACE::devices;

SerialPortDevice::SerialPortDevice(const std::string &port,
                                   std::chrono::milliseconds keepAliveInterval)
    : m_Port(port), m_SerialPortContext(std::make_unique<asio::io_context>()),
      AsyncDevice(keepAliveInterval) {}

SerialPortDevice::SerialPortDevice(const std::string &port,
                                   std::chrono::microseconds keepAliveInterval)
    : m_Port(port), m_SerialPortContext(std::make_unique<asio::io_context>()),
      AsyncDevice(keepAliveInterval) {}

void SerialPortDevice::disconnect() {
  if (m_SerialPort != nullptr && m_SerialPort->is_open()) {
    m_SerialPort->close();
  }

  AsyncDevice::disconnect();
}

void SerialPortDevice::handleAsyncConnect() {
  m_SerialPort =
      std::make_unique<asio::serial_port>(*m_SerialPortContext, m_Port);
  m_SerialPort->set_option(asio::serial_port_base::baud_rate(9600));
  m_SerialPort->set_option(asio::serial_port_base::character_size(8));
  m_SerialPort->set_option(asio::serial_port_base::stop_bits(
      asio::serial_port_base::stop_bits::one));
  m_SerialPort->set_option(
      asio::serial_port_base::parity(asio::serial_port_base::parity::none));
  m_SerialPort->set_option(asio::serial_port_base::flow_control(
      asio::serial_port_base::flow_control::none));
}

void SerialPortDevice::handleAsyncDisconnect() {
  if (m_SerialPort != nullptr && m_SerialPort->is_open()) {
    m_SerialPort->close();
  }
}

void SerialPortDevice::setFastCommunication(bool isFast) {
  auto &logger = utils::Logger::getInstance();

#if defined(_WIN32)
  // Set RTS ON
  if (!EscapeCommFunction(m_SerialPort->native_handle(),
                          isFast ? SETRTS : CLRRTS)) {
    logger.fatal("Failed to set RTS to " + std::to_string(isFast));
  } else {
    logger.info("RTS set to " + std::string(isFast ? "ON" : "OFF"));
  }
#else
  int status;
  if (ioctl(m_SerialPort->native_handle(), TIOCMGET, &status) < 0) {
    std::cerr << "Failed to get serial port status" << std::endl;
    return;
  }

  // Turn RTS ON or OFF
  if (isFast) {
    status |= TIOCM_RTS;
    logger.info("RTS set to ON");
  } else {
    status &= ~TIOCM_RTS;
    logger.info("RTS set to OFF");
  }

  if (ioctl(m_SerialPort->native_handle(), TIOCMSET, &status) < 0) {
    logger.fatal("Failed to set RTS");
  }

#endif
}

bool SerialPortDevice::operator==(const SerialPortDevice &other) const {
  return m_Port == other.m_Port;
}
