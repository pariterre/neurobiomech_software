#ifndef __STIMWALKER_DEVICES_DELSYS_EMG_DEVICE_H__
#define __STIMWALKER_DEVICES_DELSYS_EMG_DEVICE_H__

#include <array>
#include <asio.hpp>
#include <fstream>
#include <string>
#include <vector>

#include "Devices/Generic/AsyncDevice.h"
#include "Devices/Generic/DataCollector.h"
#include "Devices/Generic/TcpDevice.h"
#include "Utils/CppMacros.h"

namespace STIMWALKER_NAMESPACE::devices {

class DelsysCommands : public DeviceCommands {
public:
  static constexpr int START = 0;
  static constexpr int STOP = 1;

  DelsysCommands(int value) : DeviceCommands(value) {}

  virtual std::string toString() const {
    switch (m_Value) {
    case START:
      return "START";
    case STOP:
      return "STOP";
    default:
      return "UNKNOWN";
    }
  }
};

class DelsysEmgDevice : public AsyncDevice, public DataCollector {
public:
  /// @brief Constructor of the DelsysEmgDevice
  /// @param host The host name of the device
  /// @param commandPort The port of the command device
  /// @param dataPort The port of the data device
  DelsysEmgDevice(const std::string &host = "localhost",
                  size_t commandPort = 50040, size_t dataPort = 50043);
  DelsysEmgDevice(const DelsysEmgDevice &other) = delete;

  /// @brief Destructor of the DelsysEmgDevice
  ~DelsysEmgDevice();

  void disconnect() override;
  void startRecording() override;
  void stopRecording() override;

  /// @brief Read the data from the device
  /// @param bufferSize The size of the buffer to read
  /// @return One frame of data read from the device
  data::DataPoint read();

  /// DATA RELATED METHODS
protected:
  void handleConnect() override;

  /// @brief The command device
  DECLARE_PROTECTED_MEMBER_NOGET(TcpDevice, CommandDevice);

  /// @brief The data device
  DECLARE_PROTECTED_MEMBER_NOGET(TcpDevice, DataDevice);

  /// @brief Send a command to the [m_CommandDevice]
  /// @param command The command to send
  DeviceResponses parseCommand(const DeviceCommands &command,
                               const std::any &data) override;

  /// INTERNAL METHODS
protected:
  /// @brief The terminaison character expected by the device ("\r\n\r\n")
  DECLARE_PROTECTED_MEMBER_NOGET(std::string, TerminaisonCharacters)

  /// @brief The length of the data buffer for each channel
  DECLARE_PROTECTED_MEMBER_NOGET(size_t, BytesPerChannel)

  /// @brief The data buffer
  /// @return The data buffer
  size_t bufferSize() const;
};
} // namespace STIMWALKER_NAMESPACE::devices
#endif // __STIMWALKER_DEVICES_DELSYS_EMG_DEVICE_H__