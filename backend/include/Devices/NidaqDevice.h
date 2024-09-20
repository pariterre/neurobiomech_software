#ifndef __STIMWALKER_DEVICES_NI_DAQ_DEVICE_H__
#define __STIMWALKER_DEVICES_NI_DAQ_DEVICE_H__

#include <map>
#include <memory>
#include <mutex>
#include <thread>

#include "Devices/Generic/Collector.h"
#include "Devices/Generic/Device.h"
#include "stimwalkerConfig.h"

namespace STIMWALKER_NAMESPACE::devices {

/// @brief Abstract class for devices
class NidaqDevice : public Device, public Collector {

public:
  /// @brief Constructor
  /// @param nbChannels The number of channels
  /// @param frameRate The frame rate
  NidaqDevice(int nbChannels, int frameRate);

  // Delete copy constructor and assignment operator, this class cannot be
  // copied because of the mutex member
  NidaqDevice(const NidaqDevice &) = delete;
  NidaqDevice &operator=(const NidaqDevice &) = delete;

  virtual ~NidaqDevice();

  int getNbChannels() const override;

  int getFrameRate() const override;

  bool getIsConnected() const override;

  void connect() override;

  /// @brief Connect the device, this method is called by connect and can be
  /// overriden by a mock
  virtual void connectInternal();

  void disconnect() override;

  /// @brief Disconnect the device, this method is called by disconnect and can
  /// be overriden by a mock
  virtual void disconnectInternal();

  void dispose() override;

  bool isRecording() const override;

  void startRecording() override;

  /// @brief Start recording the data, this method is called by startRecording
  /// and can be overriden by a mock
  virtual void startRecordingInternal();

  void stopRecording() override;

  /// @brief Stop recording the data, this method is called by stopRecording and
  /// can be overriden by a mock
  virtual void stopRecordingInternal();

  int onNewData(
      std::function<void(const CollectorData &newData)> callback) override;

  void removeListener(int listenerId) override;

  std::vector<CollectorData> getData() const override;

  CollectorData getData(int index) const override;

protected:
  /// @brief Notify the listeners that new data has been collected
  void notifyListeners(const CollectorData &newData);

  bool m_isConnected; ///< Is the device connected
  bool m_isRecording =
      false; ///< Is the device currently recording (thread safe)

  int m_nbChannels; ///< Number of channels of the device
  int m_frameRate;  ///< Frame rate of the device

  std::vector<CollectorData> m_data; ///< Data collected by the device
  std::map<int, std::function<void(const CollectorData &newData)>>
      m_listeners; ///< Listeners for the data collected

  // Thread safe information while recording
  std::thread m_recordingThread; ///< Thread to simulate the recording
  std::mutex m_recordingMutex;   ///< Mutex to protect the recording state
};

class NidaqDeviceMock : public NidaqDevice {
public:
  NidaqDeviceMock(int nbChannels, int frameRate);

  // Delete copy constructor and assignment operator, this class cannot be
  // copied because of the mutex member
  NidaqDeviceMock(const NidaqDeviceMock &) = delete;
  NidaqDeviceMock &operator=(const NidaqDeviceMock &) = delete;

  void startRecordingInternal() override;

  void stopRecordingInternal() override;

protected:
  /// @brief Simulate the recording
  void generateData();
  bool m_generateData =
      false; ///< Should the mock continue generating data (thread safe)
};

} // namespace STIMWALKER_NAMESPACE::devices

#endif // __STIMWALKER_DEVICES_NI_DAQ_DEVICE_H__