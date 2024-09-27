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
class DelsysEmgDevice : public AsyncDevice, public DataCollector {
public:
  DelsysEmgDevice(std::vector<size_t> channelIndices, size_t frameRate,
                  const std::string &host = "localhost",
                  size_t commandPort = 50040, size_t dataPort = 50043);

  ~DelsysEmgDevice();

  void disconnect() override;

  void startRecording() override;

  void stopRecording() override;

  /// @brief Read the data from the device
  /// @param bufferSize The size of the buffer to read
  /// @return One frame of data read from the device
  std::vector<float> read(size_t bufferSize);

  /// DATA RELATED METHODS
protected:
  void handleConnect() override;

  /// @brief The index of the channels to collect
  DECLARE_PROTECTED_MEMBER(std::vector<size_t>, ChannelIndices)

  /// @brief The command device
  DECLARE_PROTECTED_MEMBER_NOGET(TcpDevice, CommandDevice);

  /// @brief The data device
  DECLARE_PROTECTED_MEMBER_NOGET(TcpDevice, DataDevice);

  /// @brief Send a command to the [m_CommandDevice]
  /// @param command The command to send
  void sendCommand(const std::string &command);

  virtual void HandleNewData(const DataPoint &data) override;

  /// INTERNAL METHODS
protected:
  /// @brief The terminaison character expected by the device ("\r\n\r\n")
  DECLARE_PROTECTED_MEMBER_NOGET(std::string, TerminaisonCharacters)

  /// @brief The length of the data buffer for each channel
  DECLARE_PROTECTED_MEMBER_NOGET(size_t, BytesPerChannel)

  size_t bufferSize() const;
};
} // namespace STIMWALKER_NAMESPACE::devices
#endif // __STIMWALKER_DEVICES_DELSYS_EMG_DEVICE_H__