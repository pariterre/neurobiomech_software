#ifndef __STIMWALKER_DEVICES_MAGSTIM_RAPID_DEVICE_H__
#define __STIMWALKER_DEVICES_MAGSTIM_RAPID_DEVICE_H__

#include "Devices/Generic/UsbDevice.h"
#include "MagstimRapidExceptions.h"

// https://github.com/nicolasmcnair/magpy/blob/master/magpy/magstim.py#L129
// https://github.com/nigelrogasch/MAGIC/blob/master/magstim.m#L301

namespace STIMWALKER_NAMESPACE::devices {

class MagstimRapidCommands : public DeviceCommands {
public:
  DECLARE_DEVICE_COMMAND(PRINT, 0);
  DECLARE_DEVICE_COMMAND(POKE, 1);
  DECLARE_DEVICE_COMMAND(SET_FAST_COMMUNICATION, 2);
  DECLARE_DEVICE_COMMAND(GET_TEMPERATURE, 3);
  DECLARE_DEVICE_COMMAND(ARM, 4);
  DECLARE_DEVICE_COMMAND(DISARM, 5);

  virtual std::string toString() const override {
    switch (m_Value) {
    case PRINT:
      return PRINT_AS_STRING;
    case POKE:
      return POKE_AS_STRING;
    case SET_FAST_COMMUNICATION:
      return SET_FAST_COMMUNICATION_AS_STRING;
    case GET_TEMPERATURE:
      return GET_TEMPERATURE_AS_STRING;
    case ARM:
      return ARM_AS_STRING;
    case DISARM:
      return DISARM_AS_STRING;
    default:
      throw UnknownCommandException("Unknown command in MagstimRapidCommands");
    }
  }

protected:
  MagstimRapidCommands() = delete;
  MagstimRapidCommands(int value) : DeviceCommands(value) {}
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
  MagstimRapidDevice(const MagstimRapidDevice &other) = delete;

  std::string deviceName() const override;

protected:
  /// @brief Get the armed state of the device
  /// @return The armed state of the device
  DECLARE_PROTECTED_MEMBER(bool, IsArmed)

  /// @brief Get the interval at which the device is poked when armed
  /// @return The interval at which the device is poked when armed
  DECLARE_PROTECTED_MEMBER(std::chrono::milliseconds, ArmedPokeInterval)

  /// @brief Get the interval at which the device is poked when disarmed
  /// @return The interval at which the device is poked when disarmed
  DECLARE_PROTECTED_MEMBER(std::chrono::milliseconds, DisarmedPokeInterval)

protected:
  void pingDeviceWorker() override;

  /// @brief Parse a command received from the user and send to the device
  /// @param command The command to parse
  /// @param data The data to parse
  DeviceResponses parseAsyncSendCommand(const DeviceCommands &command,
                                        const std::any &data) override;

  /// @brief Compute the CRC checksum of the data
  /// @param data The data to compute the CRC for
  /// @return The CRC checksum of the data
  std::string computeCrc(const std::string &data);

  /// @brief Change the interval at which the device is poked
  void changePokeInterval(std::chrono::milliseconds interval);
};

class MagstimRapidDeviceMock : public MagstimRapidDevice {
public:
  MagstimRapidDeviceMock(const std::string &port);

  static MagstimRapidDeviceMock FindMagstimDevice();

  std::string computeCrcInterface(const std::string &data);

protected:
  void handleAsyncConnect() override;
  void handleAsyncDisconnect() override;

  void setFastCommunication(bool isFast) override;
};

} // namespace STIMWALKER_NAMESPACE::devices
#endif // __STIMWALKER_DEVICES_MAGSTIM_RAPID_DEVICE_H__