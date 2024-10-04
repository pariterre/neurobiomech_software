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
  DECLARE_DEVICE_COMMAND(START, 0);
  DECLARE_DEVICE_COMMAND(STOP, 1);

  virtual std::string toString() const {
    switch (m_Value) {
    case START:
      return START_AS_STRING + m_TerminaisonCharacters;
    case STOP:
      return STOP_AS_STRING + m_TerminaisonCharacters;
    default:
      throw UnknownCommandException("Unknown command in DelsysCommands");
    }
  }

  /// @brief The terminaison characters for the command ("\r\n\r\n")
  DECLARE_PROTECTED_MEMBER_NOGET(std::string, TerminaisonCharacters)

protected:
  friend class DelsysEmgDevice;
  DelsysCommands(int value)
      : m_TerminaisonCharacters("\r\n\r\n"), DeviceCommands(value) {}
  DelsysCommands() = delete;
};

class DelsysEmgDevice : public AsyncDevice, public DataCollector {
protected:
  class CommandTcpDevice : public TcpDevice {
  public:
    CommandTcpDevice(const std::string &host, size_t port);
    CommandTcpDevice(const CommandTcpDevice &other) = delete;

  protected:
    DeviceResponses parseAsyncSendCommand(const DeviceCommands &command,
                                          const std::any &data) override;
  };

  class DataTcpDevice : public TcpDevice {
  public:
    DataTcpDevice(const std::string &host, size_t port);
    DataTcpDevice(const DataTcpDevice &other) = delete;

  protected:
    DeviceResponses parseAsyncSendCommand(const DeviceCommands &command,
                                          const std::any &data) override;
  };

public:
  /// @brief Constructor of the DelsysEmgDevice
  /// @param host The host name of the device
  /// @param commandPort The port of the command device
  /// @param dataPort The port of the data device
  DelsysEmgDevice(const std::string &host = "localhost",
                  size_t commandPort = 50040, size_t dataPort = 50043);
  DelsysEmgDevice(const DelsysEmgDevice &other) = delete;

protected:
  DelsysEmgDevice(std::unique_ptr<CommandTcpDevice> commandDevice,
                  std::unique_ptr<DataTcpDevice> dataDevice,
                  const std::string &host = "localhost",
                  size_t commandPort = 50040, size_t dataPort = 50043);

public:
  /// @brief Destructor of the DelsysEmgDevice
  ~DelsysEmgDevice();

  void disconnect() override;

protected:
  void handleAsyncConnect() override;
  void handleAsyncDisconnect() override;
  void handleStartRecording() override;
  void handleStopRecording() override;

  /// @brief The command device
  DECLARE_PROTECTED_MEMBER_NOGET(std::unique_ptr<CommandTcpDevice>,
                                 CommandDevice);

  /// @brief The data device
  DECLARE_PROTECTED_MEMBER_NOGET(std::unique_ptr<DataTcpDevice>, DataDevice);

  /// @brief Send a command to the [m_CommandDevice]
  /// @param command The command to send
  DeviceResponses parseAsyncSendCommand(const DeviceCommands &command,
                                        const std::any &data) override;

  /// DATA RELATED METHODS
public: // protected:
  /// @brief Read the data from the device. This is meant to be called in a loop
  /// from a worker thread
  /// @param bufferSize The size of the buffer to read
  /// @return One frame of data read from the device
  data::DataPoint readData();

protected:
  /// @brief The length of the data buffer for each channel (4 for the Delsys)
  DECLARE_PROTECTED_MEMBER(size_t, BytesPerChannel)

  /// @brief The number of channels (16 for the Delsys Trigno)
  DECLARE_PROTECTED_MEMBER(size_t, ChannelCount)

  /// @brief The snample count for each frame (27 for the Delsys)
  DECLARE_PROTECTED_MEMBER(size_t, SampleCount)

  /// @brief The buffer to read the data from the device
  DECLARE_PROTECTED_MEMBER(std::vector<char>, DataBuffer)
};

/// ------------ ///
/// MOCK SECTION ///
/// ------------ ///

class DelsysEmgDeviceMock : public DelsysEmgDevice {
protected:
  class CommandTcpDeviceMock : public CommandTcpDevice {
  public:
    CommandTcpDeviceMock(const std::string &host, size_t port);
    void read(std::vector<char> &buffer) override;

  protected:
    DeviceResponses parseAsyncSendCommand(const DeviceCommands &command,
                                     const std::any &data) override;

    void handleAsyncConnect() override;
  };

  class DataTcpDeviceMock : public DataTcpDevice {
  public:
    DataTcpDeviceMock(const std::string &host, size_t port);
    void read(std::vector<char> &buffer) override;

  protected:
    DeviceResponses parseAsyncSendCommand(const DeviceCommands &command,
                                     const std::any &data) override;

    void handleAsyncConnect() override;
  };

public:
  DelsysEmgDeviceMock(const std::string &host = "localhost",
                      size_t commandPort = 50040, size_t dataPort = 50043);
};

} // namespace STIMWALKER_NAMESPACE::devices
#endif // __STIMWALKER_DEVICES_DELSYS_EMG_DEVICE_H__