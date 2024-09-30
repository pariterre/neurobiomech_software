#ifndef __STIMWALKER_DEVICES_DATA_DATA_DEVICES_H__
#define __STIMWALKER_DEVICES_DATA_DATA_DEVICES_H__

#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <vector>

#include "Utils/CppMacros.h"
#include "stimwalkerConfig.h"

namespace STIMWALKER_NAMESPACE::devices {
namespace data {

class TimeSeries;

/// @brief Class to store data
class DataDevices {
public:
  /// @brief Get the number of devices in the collection
  /// @return The number of devices in the collection
  size_t size() const;

  /// @brief Create a new device in the collection
  /// @param deviceName The name of the device
  void newDevice(const std::string &deviceName);

  /// @brief Get the data of a specific device
  /// @param deviceName The name of the device
  /// @return The data of the device
  TimeSeries &device(const std::string &deviceName);

  /// @brief Add a new device to the collection
  /// @param deviceName The name of the device
  /// @return The data of the device
  TimeSeries &operator[](const std::string &deviceName);

  /// @brief Get the data in serialized form
  /// @return The data in serialized form
  nlohmann::json serialize() const;

  /// @brief Deserialize the data
  /// @param json The data in serialized form
  static DataDevices deserialize(const nlohmann::json &json);

protected:
  /// @brief The data of the collection
  std::map<std::string, TimeSeries> m_AllDevices;
};

} // namespace data
} // namespace STIMWALKER_NAMESPACE::devices

#endif // __STIMWALKER_DEVICES_DATA_DATA_DEVICES_H__