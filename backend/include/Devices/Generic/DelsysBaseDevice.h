#ifndef __NEUROBIO_DEVICES_DELSYS_BASE_DEVICE_H__
#define __NEUROBIO_DEVICES_DELSYS_BASE_DEVICE_H__

#include "neurobioConfig.h"

#include <array>
#include <asio.hpp>
#include <fstream>
#include <string>
#include <vector>

#include "Devices/Exceptions.h"
#include "Devices/Generic/AsyncDataCollector.h"
#include "Devices/Generic/AsyncDevice.h"
#include "Devices/Generic/TcpDevice.h"
#include "Utils/CppMacros.h"

namespace NEUROBIO_NAMESPACE::devices {
class DelsysEmgDeviceMock;
class DelsysAnalogDeviceMock;

namespace DelsysBaseDeviceMock {
class CommandTcpDeviceMock;
class DataTcpDeviceMock;
} // namespace DelsysBaseDeviceMock

class DelsysCommands : public DeviceCommands {
public:
  DECLARE_DEVICE_COMMAND(INITIALIZING, -1);
  DECLARE_DEVICE_COMMAND(START, 0);
  DECLARE_DEVICE_COMMAND(STOP, 1);
  DECLARE_DEVICE_COMMAND(SET_BACKWARD_COMPATIBILITY, 2);
  DECLARE_DEVICE_COMMAND(SET_UPSAMPLE, 3);

  virtual std::string toString() const {
    switch (m_Value) {
    case START:
      return START_AS_STRING + m_TerminaisonCharacters;
    case STOP:
      return STOP_AS_STRING + m_TerminaisonCharacters;
    case SET_BACKWARD_COMPATIBILITY:
      return "BACKWARDS COMPATIBILITY ON" + m_TerminaisonCharacters;
    case SET_UPSAMPLE:
      return "UPSAMPLE ON" + m_TerminaisonCharacters;
    default:
      throw UnknownCommandException("Unknown command in DelsysCommands");
    }
  }

  /// @brief The terminaison characters for the command ("\r\n\r\n")
  DECLARE_PROTECTED_MEMBER_NOGET(std::string, TerminaisonCharacters)

protected:
  friend class DelsysBaseDevice;
  DelsysCommands(int value)
      : m_TerminaisonCharacters("\r\n\r\n"), DeviceCommands(value) {}
  DelsysCommands() = delete;
};

class DelsysBaseDevice : public AsyncDevice, public AsyncDataCollector {
  friend devices::DelsysEmgDeviceMock;
  friend devices::DelsysAnalogDeviceMock;

public:
  class CommandTcpDevice : public TcpDevice {
  public:
    CommandTcpDevice(const std::string &host, size_t port);
    CommandTcpDevice(const CommandTcpDevice &other) = delete;

    std::string deviceName() const override;

  protected:
    DeviceResponses parseAsyncSendCommand(const DeviceCommands &command,
                                          const std::any &data) override;

  private:
    DECLARE_PROTECTED_MEMBER_NOGET(DelsysCommands, LastCommand);
    DECLARE_PROTECTED_MEMBER_NOGET(std::mutex, LastCommandMutex);
  };

  class DataTcpDevice : public TcpDevice {
  public:
    DataTcpDevice(const std::string &host, size_t port);
    DataTcpDevice(const DataTcpDevice &other) = delete;

    std::string deviceName() const override;

  protected:
    DeviceResponses parseAsyncSendCommand(const DeviceCommands &command,
                                          const std::any &data) override;
  };

public:
  /// @brief Constructor of the DelsysBaseDevice
  /// @param channelCount The number of channels of the device
  /// @param deltaTime The time between each data point (1/FrameRate)
  /// @param sampleCount Expected number of data at each block of sampled data
  /// @param host The host (ip) of the device
  /// @param dataPort The port of the data device
  /// @param commandPort The port of the command device (default 50040)
  DelsysBaseDevice(size_t channelCount, std::chrono::microseconds deltaTime,
                   size_t sampleCount, const std::string &host, size_t dataPort,
                   size_t commandPort);

  /// @brief Constructor of the DelsysBaseDevice
  /// @param channelCount The number of channels of the device
  /// @param deltaTime The time between each data point (1/FrameRate)
  /// @param sampleCount Expected number of data at each block of sampled data
  /// @param dataPort The port of the data device
  /// @param other The other DelsysBaseDevice to share the command device with
  /// and the host address
  DelsysBaseDevice(size_t channelCount, std::chrono::microseconds deltaTime,
                   size_t sampleCount, size_t dataPort,
                   const DelsysBaseDevice &other);

  DelsysBaseDevice(const DelsysBaseDevice &other) = delete;

protected:
  /// @brief Constructor of the DelsysBaseDevice that allows to pass mocker
  /// devices
  DelsysBaseDevice(std::unique_ptr<DataTcpDevice> dataDevice,
                   std::shared_ptr<CommandTcpDevice> commandDevice,
                   size_t channelCount, std::chrono::microseconds deltaTime,
                   size_t sampleCount);

public:
  /// @brief Destructor of the DelsysBaseDevice
  ~DelsysBaseDevice();

protected:
  DECLARE_PROTECTED_MEMBER(std::chrono::microseconds, DeltaTime);

  bool handleConnect() override;
  bool handleDisconnect() override;
  bool handleStartDataStreaming() override;
  bool handleStopDataStreaming() override;

  /// @brief The command device
  DECLARE_PROTECTED_MEMBER_NOGET(std::shared_ptr<CommandTcpDevice>,
                                 CommandDevice);

  /// @brief The data device
  DECLARE_PROTECTED_MEMBER_NOGET(std::unique_ptr<DataTcpDevice>, DataDevice);

  /// @brief Send a command to the [m_CommandDevice]
  /// @param command The command to send
  DeviceResponses parseAsyncSendCommand(const DeviceCommands &command,
                                        const std::any &data) override;

  /// DATA RELATED METHODS
public: // protected:
  void dataCheck() override;

protected:
  /// @brief The length of the data buffer for each channel (4 for the Delsys)
  DECLARE_PROTECTED_MEMBER(size_t, BytesPerChannel)

  /// @brief The snample count for each frame (27 for the Delsys)
  DECLARE_PROTECTED_MEMBER(size_t, SampleCount)

  /// @brief The buffer to read the data from the device
  DECLARE_PROTECTED_MEMBER(std::vector<char>, DataBuffer)
  void handleNewData(const data::DataPoint &data) override;
};

/// ------------ ///
/// MOCK SECTION ///
/// ------------ ///

namespace DelsysBaseDeviceMock {

class DelsysCommandsMock : public DelsysCommands {
public:
  DelsysCommandsMock(int value) : DelsysCommands(value) {}
  static DelsysCommandsMock fromString(const std::string &command) {
    if (command == DelsysCommandsMock(NONE).toString()) {
      return NONE;
    } else if (command == DelsysCommandsMock(START).toString()) {
      return START;
    } else if (command == DelsysCommandsMock(STOP).toString()) {
      return STOP;
    } else if (command ==
               DelsysCommandsMock(SET_BACKWARD_COMPATIBILITY).toString()) {
      return SET_BACKWARD_COMPATIBILITY;
    } else if (command == DelsysCommandsMock(SET_UPSAMPLE).toString()) {
      return SET_UPSAMPLE;
    } else {
      throw UnknownCommandException("Unknown command in DelsysCommandsMock");
    }
  }

  DECLARE_DEVICE_COMMAND(NONE, -1);

  std::string toString() const override {
    switch (m_Value) {
    case NONE:
      return NONE_AS_STRING + m_TerminaisonCharacters;
    default:
      return DelsysCommands::toString();
    }
  }
};

class CommandTcpDeviceMock : public DelsysBaseDevice::CommandTcpDevice {
public:
  CommandTcpDeviceMock(const std::string &host, size_t port);
  bool write(const std::string &data) override;
  bool read(std::vector<char> &buffer) override;

protected:
  DECLARE_PROTECTED_MEMBER_NOGET(DelsysCommandsMock, LastCommand)

  bool handleConnect() override;
};

class DataTcpDeviceMock : public DelsysBaseDevice::DataTcpDevice {
public:
  DataTcpDeviceMock(size_t channelCount, std::chrono::microseconds deltaTime,
                    size_t sampleCount, const std::string &host, size_t port);
  bool read(std::vector<char> &buffer) override;

protected:
  DeviceResponses parseAsyncSendCommand(const DeviceCommands &command,
                                        const std::any &data) override;

  DECLARE_PROTECTED_MEMBER_NOGET(size_t, DataChannelCount);
  DECLARE_PROTECTED_MEMBER_NOGET(size_t, SampleCount);

  bool handleConnect() override;

  /// @brief The time at which the data started collecting
  DECLARE_PROTECTED_MEMBER_NOGET(
      std::chrono::time_point<std::chrono::high_resolution_clock>, StartTime);
  DECLARE_PROTECTED_MEMBER_NOGET(std::chrono::microseconds, DeltaTime);

private:
  DECLARE_PRIVATE_MEMBER_NOGET(size_t, DataCounter);
};
}; // namespace DelsysBaseDeviceMock

} // namespace NEUROBIO_NAMESPACE::devices
#endif // __NEUROBIO_DEVICES_DELSYS_BASE_DEVICE_H__