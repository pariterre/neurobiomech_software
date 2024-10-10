#ifndef __STIMWALKER_DEVICES_DATA_DEVICES_DATA_H__
#define __STIMWALKER_DEVICES_DATA_DEVICES_DATA_H__

#include "stimwalkerConfig.h"

#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <vector>

namespace STIMWALKER_NAMESPACE {
namespace devices {
class DataCollector;
class Device;

/// @brief Class to store data
class Devices {
public:
  /// @brief Create a new device in the collection
  /// @param deviceName The name of the device
  void add(std::unique_ptr<Device> device);

  /// @brief Get the requested device
  /// @param deviceName The name of the device
  /// @return The requested device
  Device &getDevice(const std::string &deviceName);

  /// @brief Get the requested device
  /// @param deviceName The name of the device
  /// @return The requested device
  const Device &getDevice(const std::string &deviceName) const;

  /// @brief Get the requested data collector
  /// @param deviceName The name of the data collector
  /// @return The requested data collector
  DataCollector &getDataCollector(const std::string &deviceName);

  /// @brief Get the requested data collector
  /// @param deviceName The name of the data collector
  /// @return The requested data collector
  const DataCollector &getDataCollector(const std::string &deviceName) const;

  /// @brief Get the data in serialized form
  /// @return The data in serialized form
  nlohmann::json serialize() const;

protected:
  /// @brief The collection of devices
  std::map<std::string, std::shared_ptr<Device>> m_Devices;

  /// @brief The collection of data collectors
  std::map<std::string, std::shared_ptr<DataCollector>> m_DataCollectors;
};

} // namespace devices
} // namespace STIMWALKER_NAMESPACE

#endif // __STIMWALKER_DEVICES_DATA_DEVICES_DATA_H__