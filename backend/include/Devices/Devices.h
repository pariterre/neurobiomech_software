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
  void add(Device &device);

  /// @brief Create a new data collector in the collection
  /// @param dataCollector The data collector to add
  void add(DataCollector &dataCollector);

  /// @brief Get the data in serialized form
  /// @return The data in serialized form
  nlohmann::json serialize() const;

protected:
  /// @brief The collection of data collectors
  std::map<std::string, DataCollector &> m_DataCollectors;

  /// @brief The collection of devices
  std::map<std::string, Device &> m_Devices;
};

} // namespace devices
} // namespace STIMWALKER_NAMESPACE

#endif // __STIMWALKER_DEVICES_DATA_DEVICES_DATA_H__