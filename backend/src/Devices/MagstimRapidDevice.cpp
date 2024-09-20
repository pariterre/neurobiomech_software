#include "Devices/MagstimRapidDevice.h"

#if defined(_WIN32)
#include <cfgmgr32.h>
#include <setupapi.h>
#include <windows.h>
#else // Linux or macOS
#include <filesystem>
#include <fstream>
#endif // _WIN32
#include <regex>

using namespace STIMWALKER_NAMESPACE::devices;

MagstimRapidDevice MagstimRapidDevice::FindMagstimDevice() {
  for (const auto &device : UsbDevice::listAllUsbDevices()) {
    if (device.getVid() == "067B" && device.getPid() == "2303") {
      return MagstimRapidDevice(device.getPort());
    }
  }
  throw UsbDeviceNotFoundException("MagstimRapid device not found");
}

MagstimRapidDevice::MagstimRapidDevice(const std::string &port)
    : m_IsArmed(false), m_ArmedPokeInterval(std::chrono::milliseconds(5000)),
      m_DisarmedPokeInterval(std::chrono::milliseconds(500)),
      m_PokeInterval(std::chrono::milliseconds(-1)),
      UsbDevice(port, "067B", "2303") {}

void MagstimRapidDevice::_initialize() {
  UsbDevice::_initialize();

  // Add a keep-alive timer
  m_KeepAliveTimer = std::make_unique<asio::steady_timer>(*m_Context);
  m_PokeInterval = std::chrono::milliseconds(1000);
  _keepAlive(m_PokeInterval);
}

void MagstimRapidDevice::_parseCommand(const UsbCommands &command,
                                       const std::any &data) {
  // First call the parent class to handle the common commands
  UsbDevice::_parseCommand(command, data);

  try {
    switch (command.getValue()) {
    case MagstimRapidCommands::POKE:
      std::cout << "Sent command: " << std::any_cast<std::string>(data)
                << std::endl;
      break;

    case MagstimRapidCommands::SET_FAST_COMMUNICATION:
      _setFastCommunication(std::any_cast<bool>(data));
      break;

    case MagstimRapidCommands::ARM:
      if (m_IsArmed) {
        throw MagsimRapidAlreadyArmedException("The device is already armed");
      }
      m_IsArmed = command.getValue() == MagstimRapidCommands::ARM;

      _changePokeInterval(std::chrono::milliseconds(
          m_IsArmed ? m_ArmedPokeInterval : m_DisarmedPokeInterval));

      std::cout << (m_IsArmed ? "Armed" : "Disarmed")
                << " the system and changed poke interval to "
                << m_PokeInterval.count() << " ms" << std::endl;
      break;

    case MagstimRapidCommands::DISARM:
      if (!m_IsArmed) {
        throw MagsimRapidNotArmedException("The device is already disarmed");
      }

      m_IsArmed = command.getValue() == MagstimRapidCommands::ARM;

      _changePokeInterval(std::chrono::milliseconds(
          m_IsArmed ? m_ArmedPokeInterval : m_DisarmedPokeInterval));

      std::cout << (m_IsArmed ? "Armed" : "Disarmed")
                << " the system and changed poke interval to "
                << m_PokeInterval.count() << " ms" << std::endl;
      break;
    }
  } catch (const std::bad_any_cast &) {
    std::cerr << "The data you provided with the command ("
              << command.getValue() << ") is invalid" << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }

  // Send a command to the USB device
  // asio::write(*m_SerialPort, asio::buffer(command));
}

void MagstimRapidDevice::_keepAlive(const std::chrono::milliseconds &timeout) {
  // Set a 5-second timer
  m_KeepAliveTimer->expires_after(timeout);

  m_KeepAliveTimer->async_wait([this](const asio::error_code &ec) {
    // If ec is not false, it means the timer was stopped to change the
    // interval, or the device was disconnected. In both cases, do nothing and
    // return
    if (ec)
      return;
    // Otherwise, send a PING command to the device
    std::lock_guard<std::mutex> lock(m_Mutex);
    _parseCommand(MagstimRapidCommands::POKE, std::string("POKE"));
    _keepAlive(m_PokeInterval);
  });
}

void MagstimRapidDevice::_changePokeInterval(
    std::chrono::milliseconds interval) {
  // Stop the timer

  // Compute the remaining time before the next PING command was supposed to be
  // sent
  auto remainingTime = std::chrono::duration_cast<std::chrono::milliseconds>(
      m_KeepAliveTimer->expiry() - asio::steady_timer::clock_type::now());
  auto elapsedTime = m_PokeInterval - remainingTime;

  // Set the interval to the requested value
  m_PokeInterval = interval;

  // Send a keep alive command with the remaining time
  m_KeepAliveTimer->cancel();
  _keepAlive(interval - elapsedTime);
}

// --- MOCKER SECTION --- //
MagstimRapidDeviceMock::MagstimRapidDeviceMock(const std::string &port)
    : MagstimRapidDevice(port) {}

MagstimRapidDeviceMock MagstimRapidDeviceMock::FindMagstimDevice() {
  return MagstimRapidDeviceMock("MOCK");
}

void MagstimRapidDeviceMock::_connectSerialPort() {
  // Do nothing
}
