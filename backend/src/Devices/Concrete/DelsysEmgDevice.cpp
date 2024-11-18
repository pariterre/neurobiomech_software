#include "Devices/Concrete/DelsysEmgDevice.h"

#include <thread>

using namespace STIMWALKER_NAMESPACE::devices;

size_t DELSYS_EMG_CHANNEL_COUNT(16);
size_t DELSYS_EMG_ACQUISITION_FREQUENCY(2000);
std::chrono::microseconds
    DELSYS_EMG_FRAME_RATE(1000 * 1000 * 1 / DELSYS_EMG_ACQUISITION_FREQUENCY);
size_t DELSYS_EMG_SAMPLE_COUNT(27);

DelsysEmgDevice::DelsysEmgDevice(const std::string &host, size_t dataPort,
                                 size_t commandPort)
    : DelsysBaseDevice(DELSYS_EMG_CHANNEL_COUNT, DELSYS_EMG_FRAME_RATE,
                       DELSYS_EMG_SAMPLE_COUNT, host, dataPort, commandPort) {}

DelsysEmgDevice::DelsysEmgDevice(const DelsysBaseDevice &other, size_t dataPort)
    : DelsysBaseDevice(DELSYS_EMG_CHANNEL_COUNT, DELSYS_EMG_FRAME_RATE,
                       DELSYS_EMG_SAMPLE_COUNT, dataPort, other) {}

DelsysEmgDevice::DelsysEmgDevice(
    std::unique_ptr<DataTcpDevice> dataDevice,
    std::shared_ptr<CommandTcpDevice> commandDevice)
    : DelsysBaseDevice(std::move(dataDevice), commandDevice,
                       DELSYS_EMG_CHANNEL_COUNT, DELSYS_EMG_FRAME_RATE,
                       DELSYS_EMG_SAMPLE_COUNT) {}

std::string DelsysEmgDevice::deviceName() const { return "DelsysEmgDevice"; }

std::string DelsysEmgDevice::dataCollectorName() const {
  return "DelsysEmgDataCollector";
}

/// ------------ ///
/// MOCK SECTION ///
/// ------------ ///
using namespace DelsysBaseDeviceMock;
DelsysEmgDeviceMock::DelsysEmgDeviceMock(const std::string &host,
                                         size_t dataPort, size_t commandPort)
    : DelsysEmgDevice(
          std::make_unique<DataTcpDeviceMock>(
              DELSYS_EMG_CHANNEL_COUNT, DELSYS_EMG_FRAME_RATE,
              DELSYS_EMG_SAMPLE_COUNT, host, dataPort),
          std::make_shared<CommandTcpDeviceMock>(host, commandPort)) {}

DelsysEmgDeviceMock::DelsysEmgDeviceMock(const DelsysBaseDevice &other,
                                         size_t dataPort)
    : DelsysEmgDevice(std::make_unique<DataTcpDeviceMock>(
                          DELSYS_EMG_CHANNEL_COUNT, DELSYS_EMG_FRAME_RATE,
                          DELSYS_EMG_SAMPLE_COUNT,
                          other.m_CommandDevice->getHost(), dataPort),
                      other.m_CommandDevice) {}

bool DelsysEmgDeviceMock::handleConnect() {
  if (shouldFailToConnect) {
    // Simulate a failure to connect after few time
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    return false;
  }
  return DelsysEmgDevice::handleConnect();
}

bool DelsysEmgDeviceMock::handleStartDataStreaming() {
  if (shouldFailToStartDataStreaming) {
    // Simulate a failure to connect after few time
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    return false;
  }
  return DelsysEmgDevice::handleStartDataStreaming();
}
