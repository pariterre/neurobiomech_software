#ifndef __STIMWALKER_DEVICES_DATA_DEVICES_DATA_H__
#define __STIMWALKER_DEVICES_DATA_DEVICES_DATA_H__

#include "stimwalkerConfig.h"

#include "Utils/CppMacros.h"
#include <nlohmann/json.hpp>

namespace STIMWALKER_NAMESPACE {
namespace devices {
class DataCollector;
class Device;

/// @brief Class to store data
class Devices {
public:
  /// @brief Create a new device in the collection
  /// @param device The device to add
  /// @return The id of the device in the collection so it can be accessed or
  /// removed later
  int add(std::unique_ptr<Device> device);

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

  /// @brief Get the data in serialized form
  /// @return The data in serialized form
  nlohmann::json serialize() const;

protected:
  /// @brief The collection of devices
  std::map<int, std::shared_ptr<Device>> m_Devices;

  /// @brief The collection of data collectors
  std::map<int, std::shared_ptr<DataCollector>> m_DataCollectors;
};

} // namespace devices
} // namespace STIMWALKER_NAMESPACE

#endif // __STIMWALKER_DEVICES_DATA_DEVICES_DATA_H__