#ifndef __STIMWALKER_DEVICES_DATA_DEVICES_DATA_H__
#define __STIMWALKER_DEVICES_DATA_DEVICES_DATA_H__

#include "stimwalkerConfig.h"

#include "Utils/CppMacros.h"
#include <nlohmann/json.hpp>

namespace STIMWALKER_NAMESPACE {
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
  /// @brief Create a new device in the collection
  /// @param device The device to add
  /// @return The id of the device in the collection so it can be accessed or
  /// removed later
  int add(std::unique_ptr<Device> device);

  /// @brief Remove the device from the collection
  /// @param deviceId The id of the device (the one returned by the add method)
  void remove(int deviceId);

  /// @brief Remove all the devices from the collection
  void clear();

  /// @brief Get the requested device
  /// @param deviceId The id of the device (the one returned by the add method)
  /// @return The requested device
  Device &getDevice(int deviceId);

  /// @brief Get the requested device
  /// @param deviceId The id of the device (the one returned by the add method)
  /// @return The requested device
  const Device &getDevice(int deviceId) const;

  /// @brief Get the requested data collector
  /// @param deviceId The id of the data collector
  /// @return The requested data collector
  DataCollector &getDataCollector(int deviceId);

  /// @brief Get the requested data collector
  /// @param deviceId The id of the data collector
  /// @return The requested data collector
  const DataCollector &getDataCollector(int deviceId) const;

  /// DRIVING THE DEVICES METHODS ///
public:
  /// @brief Connect all the devices in a blocking way (wait for all the devices
  /// to be connected before returning)
  void connect();

  /// @brief Disconnect all the devices in a blocking way (wait for all the
  /// devices to be disconnected before returning)
  void disconnect();

  /// @brief Start recording on all the devices in a blocking way (wait for all
  /// the devices to start recording before returning). Additionally, it sets
  /// all the starting reconding time of all device to "now" as of the time of
  /// the when all the devices are started
  void startRecording();

  /// @brief Stop recording on all the devices in a blocking way (wait for all
  /// the devices to stop recording before returning)
  void stopRecording();

  /// @brief Get the data from all the devices
  /// @return The data from all the devices in a map with the device id as the
  /// key
  std::map<int, data::TimeSeries> getTrialData() const;

  /// DATA SPECIFIC METHODS ///
public:
  /// @brief Get the data in serialized form
  /// @return The data in serialized form
  nlohmann::json serialize() const;

  /// INTERNAL ///
protected:
  /// @brief If the devices are connected
  DECLARE_PROTECTED_MEMBER(bool, IsConnected)

  /// @brief If the devices are recording
  DECLARE_PROTECTED_MEMBER(bool, IsRecording)

protected:
  /// @brief The collection of devices
  std::map<int, std::shared_ptr<Device>> m_Devices;

  /// @brief The collection of data collectors
  std::map<int, std::shared_ptr<DataCollector>> m_DataCollectors;
};

} // namespace devices
} // namespace STIMWALKER_NAMESPACE

#endif // __STIMWALKER_DEVICES_DATA_DEVICES_DATA_H__