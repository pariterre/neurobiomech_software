#include "Devices/Concrete/DelsysEmgDevice.h"

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

DelsysEmgDevice::DelsysEmgDevice(
    std::unique_ptr<CommandTcpDevice> commandDevice,
    std::unique_ptr<DataTcpDevice> dataDevice)
    : DelsysBaseDevice(std::move(commandDevice), std::move(dataDevice),
                       DELSYS_EMG_CHANNEL_COUNT, DELSYS_EMG_FRAME_RATE,
                       DELSYS_EMG_SAMPLE_COUNT) {}

DelsysEmgDevice::~DelsysEmgDevice() {
  stopDataCollectorWorkers();
  stopDeviceWorkers();
}

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
    : DelsysEmgDevice(std::make_unique<CommandTcpDeviceMock>(host, commandPort),
                      std::make_unique<DataTcpDeviceMock>(
                          DELSYS_EMG_CHANNEL_COUNT, DELSYS_EMG_FRAME_RATE,
                          DELSYS_EMG_SAMPLE_COUNT, host, dataPort)) {}

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
