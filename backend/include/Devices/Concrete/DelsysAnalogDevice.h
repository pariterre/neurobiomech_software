#ifndef __STIMWALKER_DEVICES_DELSYS_ANALOG_DEVICE_H__
#define __STIMWALKER_DEVICES_DELSYS_ANALOG_DEVICE_H__

#include "stimwalkerConfig.h"

#include "Devices/Generic/DelsysBaseDevice.h"

namespace STIMWALKER_NAMESPACE::devices {

class DelsysAnalogDevice : public DelsysBaseDevice {
public:
  /// @brief Constructor of the DelsysAnalogDevice
  /// @param host The host name of the device
  /// @param dataPort The port of the data device
  /// @param commandPort The port of the command device
  DelsysAnalogDevice(const std::string &host = "localhost",
                     size_t dataPort = 50044, size_t commandPort = 50040);
  DelsysAnalogDevice(const DelsysAnalogDevice &other) = delete;

protected:
  DelsysAnalogDevice(std::unique_ptr<CommandTcpDevice> commandDevice,
                     std::unique_ptr<DataTcpDevice> dataDevice);

public:
  /// @brief Destructor of the DelsysAnalogDevice
  ~DelsysAnalogDevice();

  std::string deviceName() const override;
  std::string dataCollectorName() const override;
};

/// ------------ ///
/// MOCK SECTION ///
/// ------------ ///

class DelsysAnalogDeviceMock : public devices::DelsysAnalogDevice {
public:
  DelsysAnalogDeviceMock(const std::string &host = "localhost",
                         size_t dataPort = 50042, size_t commandPort = 50040);

  bool shouldFailToConnect = false;
  bool shouldFailToStartDataStreaming = false;

protected:
  bool handleConnect() override;
  bool handleStartDataStreaming() override;
};

} // namespace STIMWALKER_NAMESPACE::devices
#endif // __STIMWALKER_DEVICES_DELSYS_ANALOG_DEVICE_H__