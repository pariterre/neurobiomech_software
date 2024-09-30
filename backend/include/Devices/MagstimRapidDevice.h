#ifndef __STIMWALKER_DEVICES_MAGSTIM_RAPID_DEVICE_H__
#define __STIMWALKER_DEVICES_MAGSTIM_RAPID_DEVICE_H__

#include "Devices/Generic/UsbDevice.h"
#include "MagstimRapidExceptions.h"

// https://github.com/nicolasmcnair/magpy/blob/master/magpy/magstim.py#L129
// https://github.com/nigelrogasch/MAGIC/blob/master/magstim.m#L301

namespace STIMWALKER_NAMESPACE::devices {

class MagstimRapidCommands : public UsbCommands {
public:
  static constexpr int POKE = 100;
  static constexpr int SET_FAST_COMMUNICATION = 101;
  static constexpr int GET_TEMPERATURE = 102;
  static constexpr int ARM = 103;
  static constexpr int DISARM = 104;

  virtual std::string toString() const override {
    auto value = UsbCommands::toString();
    if (value != "UNKNOWN") {
      return value;
    }

    switch (m_Value) {
    case POKE:
      return "POKE";
    case SET_FAST_COMMUNICATION:
      return "SET_FAST_COMMUNICATION";
    case GET_TEMPERATURE:
      return "GET_TEMPERATURE";
    case ARM:
      return "ARM";
    case DISARM:
      return "DISARM";
    default:
      return "UNKNOWN";
    }
  }

  MagstimRapidCommands() = delete;
  MagstimRapidCommands(int value) : UsbCommands(value) {}
};

/// @brief A class representing a Magstim Rapid device
/// @details This class provides a way to connect to a Magstim Rapid device and
/// send commands to it
class MagstimRapidDevice : public UsbDevice {
  /// Enums
public:
  /// Constructors
public:
  /// @brief Constructor when the port is unknown (will scan for it)
  static MagstimRapidDevice FindMagstimDevice();

  /// @brief Constructor for a known port
  /// @param port The port name of the device
  MagstimRapidDevice(const std::string &port);

protected:
  /// Protected members without Get accessors

  /// @brief Get the armed state of the device
  /// @return The armed state of the device
  DECLARE_PROTECTED_MEMBER(bool, IsArmed)

  /// @brief Get the interval at which the device is poked when armed
  /// @return The interval at which the device is poked when armed
  DECLARE_PROTECTED_MEMBER(std::chrono::milliseconds, ArmedPokeInterval)

  /// @brief Get the interval at which the device is poked when disarmed
  /// @return The interval at which the device is poked when disarmed
  DECLARE_PROTECTED_MEMBER(std::chrono::milliseconds, DisarmedPokeInterval)

  /// @brief Get how long to wait before sending the PING command
  /// @return How long to wait before sending the PING command
  DECLARE_PROTECTED_MEMBER(std::chrono::milliseconds, PokeInterval)

  /// @brief Get the keep-alive timer
  /// @return The keep-alive timer
  DECLARE_PROTECTED_MEMBER_NOGET(std::unique_ptr<asio::steady_timer>,
                                 KeepAliveTimer)

protected:
  /// @brief Connect to the ubs device. This is expected to run on an async
  /// thread
  void handleConnect() override;

  /// @brief Parse a command received from the user and send to the device
  /// @param command The command to parse
  /// @param data The data to parse
  DeviceResponses parseCommand(const DeviceCommands &command,
                               const std::any &data) override;

  /// @brief Set a worker thread to keep the device alive
  void keepAlive(const std::chrono::milliseconds &timeout);

  /// @brief Compute the CRC checksum of the data
  /// @param data The data to compute the CRC for
  /// @return The CRC checksum of the data
  std::string computeCRC(const std::string &data);

  /// @brief Change the interval at which the device is poked
  void changePokeInterval(std::chrono::milliseconds interval);
};

class MagstimRapidDeviceMock : public MagstimRapidDevice {
public:
  MagstimRapidDeviceMock(const std::string &port);

  static MagstimRapidDeviceMock FindMagstimDevice();
};

} // namespace STIMWALKER_NAMESPACE::devices
#endif // __STIMWALKER_DEVICES_MAGSTIM_RAPID_DEVICE_H__