#ifndef __STIMWALKER_DEVICES_DELSYS_EMG_DEVICE_H__
#define __STIMWALKER_DEVICES_DELSYS_EMG_DEVICE_H__

#include "stimwalkerConfig.h"

#include "Devices/Generic/DelsysBaseDevice.h"

namespace STIMWALKER_NAMESPACE::devices {

class DelsysEmgDevice : public DelsysBaseDevice {

public:
  /// @brief Constructor of the DelsysEmgDevice
  /// @param host The host name of the device
  /// @param dataPort The port of the data device
  /// @param commandPort The port of the command device
  DelsysEmgDevice(const std::string &host = "localhost",
                  size_t dataPort = 50041, size_t commandPort = 50040);

  /// @brief Constructor that share some parts of the device, namely the command
  /// device and host address
  /// @param other The other DelsysBaseDevice to share the command device with
  /// @param dataPort The port of the data device
  DelsysEmgDevice(const DelsysBaseDevice &other, size_t dataPort = 50041);

  DelsysEmgDevice(const DelsysEmgDevice &other) = delete;

protected:
  DelsysEmgDevice(std::unique_ptr<DataTcpDevice> dataDevice,
                  std::shared_ptr<CommandTcpDevice> commandDevice);

public:
  std::string deviceName() const override;
  std::string dataCollectorName() const override;
};

/// ------------ ///
/// MOCK SECTION ///
/// ------------ ///

class DelsysEmgDeviceMock : public devices::DelsysEmgDevice {
public:
  DelsysEmgDeviceMock(const std::string &host = "localhost",
                      size_t dataPort = 50041, size_t commandPort = 50040);

  /// @brief Constructor that share some parts of the device, namely the command
  /// device and host address
  /// @param other The other DelsysBaseDevice to share the command device with
  /// @param dataPort The port of the data device
  DelsysEmgDeviceMock(const DelsysBaseDevice &other, size_t dataPort = 50041);

  bool shouldFailToConnect = false;
  bool shouldFailToStartDataStreaming = false;

protected:
  bool handleConnect() override;
  bool handleStartDataStreaming() override;
};

} // namespace STIMWALKER_NAMESPACE::devices
#endif // __STIMWALKER_DEVICES_DELSYS_EMG_DEVICE_H__