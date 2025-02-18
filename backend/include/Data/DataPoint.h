#ifndef __NEUROBIO_DATA_DATA_POINT_H__
#define __NEUROBIO_DATA_DATA_POINT_H__

#include "neurobioConfig.h"

#include "Utils/CppMacros.h"
#include <map>
#include <nlohmann/json.hpp>
#include <variant>

namespace NEUROBIO_NAMESPACE::data {

using ExtraInfo =
    std::map<std::string, std::variant<size_t, int, double, bool, std::string>>;

/// @brief Class to store data
class DataPoint {

public:
  /// @brief Empty constructor
  DataPoint();

  /// @brief Contructor
  /// @param timeStamp The time stamp (i.e. elapsed time since starting time)
  /// @param data The data to store
  DataPoint(const std::chrono::microseconds &timeStamp,
            const std::vector<double> &data, const ExtraInfo &extraInfo = {});

  /// @brief Deserialize a json object
  /// @param json The json object to deserialize
  DataPoint(const nlohmann::json &json);

  /// @brief Destructor of the DataPoint
  virtual ~DataPoint() = default;

  /// @brief Get the number of channels
  /// @return The number of channels
  size_t size() const;

  /// @brief Get the specific data
  /// @param index The index of the data
  /// @return The data at the given index
  double operator[](size_t index) const;

  /// @brief Convert the object to JSON
  /// @return The JSON object
  nlohmann::json serialize() const;

protected:
  /// @brief The time stamp of the data
  DECLARE_PROTECTED_MEMBER(std::chrono::microseconds, TimeStamp);

  /// @brief The data
  DECLARE_PROTECTED_MEMBER(std::vector<double>, Data);

protected:
  /// @brief Extra information about the data
  DECLARE_PROTECTED_MEMBER(ExtraInfo, ExtraInfo);
};

} // namespace NEUROBIO_NAMESPACE::data

#endif // __NEUROBIO_DATA_DATA_POINT_H__