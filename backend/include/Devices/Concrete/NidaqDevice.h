#ifndef __STIMWALKER_DEVICES_NI_DAQ_DEVICE_H__
#define __STIMWALKER_DEVICES_NI_DAQ_DEVICE_H__

#include "stimwalkerConfig.h"

#include <map>
#include <memory>
#include <mutex>
#include <thread>

#include "Devices/Generic/AsyncDevice.h"
#include "Devices/Generic/DataCollector.h"

namespace STIMWALKER_NAMESPACE::devices {

/// @brief Abstract class for devices
class NidaqDevice : public AsyncDevice, public DataCollector {
  // TODO Change DataCollector to AsyncDataCollector

public:
  /// @brief Constructor
  /// @param channelCount The number of channels
  /// @param dataCheckIntervals The interval to check for new data
  NidaqDevice(size_t channelCount,
              const std::chrono::microseconds &dataCheckIntervals);

  // Delete copy constructor and assignment operator, this class cannot be
  // copied because of the mutex member
  NidaqDevice(const NidaqDevice &) = delete;
  NidaqDevice &operator=(const NidaqDevice &) = delete;

  std::string deviceName() const override;
  std::string dataCollectorName() const override;

  ~NidaqDevice();

protected:
  bool handleConnect() override;
  bool handleDisconnect() override;
  bool handleStartDataStreaming() override;
  bool handleStopDataStreaming() override;
  void handleNewData(const data::DataPoint &data) override;

  DeviceResponses parseAsyncSendCommand(const DeviceCommands &command,
                                        const std::any &data) override;
};

// // TODO Reimplement the mocker
// class NidaqDeviceMock : public NidaqDevice {
// public:
//   NidaqDeviceMock(int channelCount, int frameRate);

//   // Delete copy constructor and assignment operator, this class cannot be
//   // copied because of the mutex member
//   NidaqDeviceMock(const NidaqDeviceMock &) = delete;
//   NidaqDeviceMock &operator=(const NidaqDeviceMock &) = delete;

//   void startRecording() override;
//   void stopRecording() override;

// protected:
//   /// @brief Simulate the recording
//   void generateData();

//   ///< Should the mock continue generating data (thread safe)
//   bool m_generateData = false;
// };

} // namespace STIMWALKER_NAMESPACE::devices

#endif // __STIMWALKER_DEVICES_NI_DAQ_DEVICE_H__