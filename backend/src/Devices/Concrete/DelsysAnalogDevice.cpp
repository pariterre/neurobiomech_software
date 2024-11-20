#include "Devices/Concrete/DelsysAnalogDevice.h"

#include <thread>

using namespace STIMWALKER_NAMESPACE::devices;

size_t DELSYS_ANALOG_CHANNEL_COUNT(3 * 16);
std::chrono::microseconds
    DELSYS_ANALOG_FRAME_RATE(6750); // 1 / (0.0135 / 2 s in us)
size_t DELSYS_ANALOG_SAMPLE_COUNT(2);

DelsysAnalogDevice::DelsysAnalogDevice(const std::string &host, size_t dataPort,
                                       size_t commandPort)
    : DelsysBaseDevice(DELSYS_ANALOG_CHANNEL_COUNT, DELSYS_ANALOG_FRAME_RATE,
                       DELSYS_ANALOG_SAMPLE_COUNT, host, dataPort,
                       commandPort) {}

DelsysAnalogDevice::DelsysAnalogDevice(const DelsysBaseDevice &other,
                                       size_t dataPort)
    : DelsysBaseDevice(DELSYS_ANALOG_CHANNEL_COUNT, DELSYS_ANALOG_FRAME_RATE,
                       DELSYS_ANALOG_SAMPLE_COUNT, dataPort, other) {}

DelsysAnalogDevice::DelsysAnalogDevice(
    std::unique_ptr<DataTcpDevice> dataDevice,
    std::shared_ptr<CommandTcpDevice> commandDevice)
    : DelsysBaseDevice(std::move(dataDevice), commandDevice,
                       DELSYS_ANALOG_CHANNEL_COUNT, DELSYS_ANALOG_FRAME_RATE,
                       DELSYS_ANALOG_SAMPLE_COUNT) {}

std::string DelsysAnalogDevice::deviceName() const {
  return "DelsysAnalogDevice";
}

std::string DelsysAnalogDevice::dataCollectorName() const {
  return "DelsysAnalogDataCollector";
}

/// ------------ ///
/// MOCK SECTION ///
/// ------------ ///
using namespace DelsysBaseDeviceMock;
DelsysAnalogDeviceMock::DelsysAnalogDeviceMock(const std::string &host,
                                               size_t dataPort,
                                               size_t commandPort)
    : DelsysAnalogDevice(
          std::make_unique<DataTcpDeviceMock>(
              DELSYS_ANALOG_CHANNEL_COUNT, DELSYS_ANALOG_FRAME_RATE,
              DELSYS_ANALOG_SAMPLE_COUNT, host, dataPort),
          std::make_unique<CommandTcpDeviceMock>(host, commandPort)) {}

DelsysAnalogDeviceMock::DelsysAnalogDeviceMock(const DelsysBaseDevice &other,
                                               size_t dataPort)
    : DelsysAnalogDevice(std::make_unique<DataTcpDeviceMock>(
                             DELSYS_ANALOG_CHANNEL_COUNT,
                             DELSYS_ANALOG_FRAME_RATE,
                             DELSYS_ANALOG_SAMPLE_COUNT,
                             other.m_CommandDevice->getHost(), dataPort),
                         other.m_CommandDevice) {}

bool DelsysAnalogDeviceMock::handleConnect() {
  if (shouldFailToConnect) {
    // Simulate a failure to connect after few time
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    return false;
  }
  return DelsysAnalogDevice::handleConnect();
}

bool DelsysAnalogDeviceMock::handleStartDataStreaming() {
  if (shouldFailToStartDataStreaming) {
    // Simulate a failure to connect after few time
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    return false;
  }
  return DelsysAnalogDevice::handleStartDataStreaming();
}
