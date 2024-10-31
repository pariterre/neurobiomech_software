#include "Devices/Concrete/DelsysAnalogDevice.h"

using namespace STIMWALKER_NAMESPACE::devices;

size_t CHANNEL_COUNT(48);
size_t ACQUISITION_FREQUENCY(148);
std::chrono::microseconds FRAME_RATE(1 / ACQUISITION_FREQUENCY * 1000 * 1000);

DelsysAnalogDevice::DelsysAnalogDevice(const std::string &host, size_t dataPort,
                                       size_t commandPort)
    : DelsysBaseDevice(CHANNEL_COUNT, FRAME_RATE, host, dataPort, commandPort) {
}

DelsysAnalogDevice::DelsysAnalogDevice(
    std::unique_ptr<CommandTcpDevice> commandDevice,
    std::unique_ptr<DataTcpDevice> dataDevice)
    : DelsysBaseDevice(std::move(commandDevice), std::move(dataDevice),
                       CHANNEL_COUNT, FRAME_RATE) {}

DelsysAnalogDevice::~DelsysAnalogDevice() {
  stopDataCollectorWorkers();
  stopDeviceWorkers();
}

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
          std::make_unique<CommandTcpDeviceMock>(host, commandPort),
          std::make_unique<DataTcpDeviceMock>(CHANNEL_COUNT, FRAME_RATE, host,
                                              dataPort)) {}

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
