#ifndef __NEUROBIO_DEVICES_DATA_DEVICES_DATA_H__
#define __NEUROBIO_DEVICES_DATA_DEVICES_DATA_H__

#include "neurobioConfig.h"

#include "Utils/CppMacros.h"
#include <mutex>
#include <nlohmann/json.hpp>

namespace NEUROBIO_NAMESPACE {
namespace data {
class TimeSeries;
} // namespace data

namespace devices {
class DataCollector;
class Device;

/// @brief Class to store data
class Devices {

  /// DEVICE MANAGEMENT METHODS ///
public:
  /// @brief Constructor
  Devices() = default;
  ~Devices();

  /// @brief Create a new device in the collection
  /// @param device The device to add
  /// @return The id of the device in the collection so it can be accessed or
  /// removed later
  size_t add(std::unique_ptr<Device> device);

  /// @brief Set the zero level of the device
  /// @param deviceName The name of the device to set the zero level
  bool zeroLevelDevice(const std::string &deviceName);

  /// @brief Remove the device from the collection
  /// @param deviceId The id of the device (the one returned by the add method)
  void remove(size_t deviceId);

  /// @brief Get the ids of the devices in the collection
  /// @return The ids of the devices in the collection
  std::vector<size_t> getDeviceIds() const;

  /// @brief Get the names of the devices in the collection
  /// @return The names of the devices in the collection
  std::vector<std::string> getDeviceNames() const;

  /// @brief Get the number of devices in the collection
  /// @return The number of devices in the collection
  size_t size() const;

  /// @brief Remove all the devices from the collection
  void clear();

  /// @brief Get the requested device
  /// @param deviceId The id of the device (the one returned by the add method)
  /// @return The requested device
  const Device &operator[](size_t deviceId) const;

  /// @brief Get the requested device
  /// @param deviceId The id of the device (the one returned by the add method)
  /// @return The requested device
  const Device &getDevice(size_t deviceId) const;

  /// @brief Get the requested data collector
  /// @param deviceId The id of the data collector
  /// @return The requested data collector
  const DataCollector &getDataCollector(size_t deviceId) const;

  /// DRIVING THE DEVICES METHODS ///
public:
  /// @brief Connect all the devices in a blocking way (wait for all the devices
  /// to be connected before returning)
  bool connect();

  /// @brief Disconnect all the devices in a blocking way (wait for all the
  /// devices to be disconnected before returning)
  bool disconnect();

  /// @brief Start data streaming on all the devices in a blocking way (wait for
  /// all the devices to start streaming data before returning). This must be
  /// called before starting to record data
  /// @return True if all the devices started streaming data, false otherwise
  bool startDataStreaming();

  /// @brief Stop data streaming on all the devices in a blocking way (wait for
  /// all the devices to stop streaming data before returning). If the devices
  /// were recording data, this will also stop the recording
  bool stopDataStreaming();

  /// @brief Start recording on all the devices in a blocking way (wait for all
  /// the devices to start recording before returning)
  /// @return True if all the devices started recording data, false otherwise
  bool startRecording();

  /// @brief Stop recording on all the devices in a blocking way (wait for all
  /// the devices to stop recording before returning)
  /// @return True if all the devices stopped recording data, false otherwise
  bool stopRecording();

  /// DATA SPECIFIC METHODS ///
public:
  /// @brief Get the live data
  /// @return The live data
  std::map<std::string, data::TimeSeries> getLiveData() const;

  /// @brief Get the live data in serialized form
  /// @return The live datain serialized form
  nlohmann::json getLiveDataSerialized() const;

  /// @brief Get the data of the last recorded trial in serialized form
  /// @return The data of the last recorded trial in serialized form
  nlohmann::json getLastTrialDataSerialized() const;

  /// @brief Deserialize timeseries data. This is almost the opposite of
  /// serialized with the difference that the map is not a map of devices, but
  /// a map device names
  /// @param json The serialized data
  /// @return The deserialized data
  static std::map<std::string, data::TimeSeries>
  deserializeData(const nlohmann::json &json);

  /// INTERNAL ///
protected:
  /// @brief If the devices are connected
  DECLARE_PROTECTED_MEMBER(bool, IsConnected)

  /// @brief If the devices are streaming data
  DECLARE_PROTECTED_MEMBER(bool, IsStreamingData)

  /// @brief If the devices are recording
  DECLARE_PROTECTED_MEMBER(bool, IsRecording)

protected:
  /// @brief The collection of devices
  std::map<size_t, std::shared_ptr<Device>> m_Devices;

public:
  /// @brief The collection of data collectors
  const std::map<size_t, std::shared_ptr<DataCollector>> &
  getDataCollectors() const {
    return m_DataCollectors;
  }
  std::map<size_t, std::shared_ptr<DataCollector>> m_DataCollectors;

private:
  /// @brief The mutex to lock certain operations
  DECLARE_PRIVATE_MEMBER_NOGET(std::mutex, MutexDataCollectors)
};

} // namespace devices
} // namespace NEUROBIO_NAMESPACE

#endif // __NEUROBIO_DEVICES_DATA_DEVICES_DATA_H__