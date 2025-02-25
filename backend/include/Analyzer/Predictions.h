#ifndef __NEUROBIO_ANALYZER_PREDICTIONS_H__
#define __NEUROBIO_ANALYZER_PREDICTIONS_H__

#include "neurobioConfig.h"

#include "Utils/CppMacros.h"
#include "nlohmann/json.hpp"
#include <map>
#include <memory>

namespace NEUROBIO_NAMESPACE::data {
class DataPoint;
} // namespace NEUROBIO_NAMESPACE::data

namespace NEUROBIO_NAMESPACE::analyzer {

class Predictions {
public:
  /// @brief Constructor of the Predictions
  Predictions();

  /// @brief Constructor of the Predictions from a json object
  /// @param json The json object to create the predictions from
  Predictions(const nlohmann::json &json);

  /// @brief Destructor of the Predictions
  ~Predictions() = default;

public:
  /// @brief Add a prediction to the prediction set
  /// @param name The name of the prediction
  void add(const std::string &name);

  /// @brief Set the prediction
  /// @param name The name of the prediction
  /// @param value The value of the prediction
  void set(const std::string &name, const data::DataPoint &value);

  /// @brief Remove a prediction from the prediction set
  /// @param name The name of the prediction
  void remove(const std::string &name);

  /// @brief Get the number of predictions in the collection
  /// @return The number of predictions in the collection
  size_t size() const;

  /// @brief Remove all the predictions from the collection
  void reset();

  /// @brief Serialize the predictions
  /// @return The serialized predictions
  nlohmann::json serialize() const;

  /// @brief Get the requested prediction
  /// @param name The name of the prediction
  /// @return The requested prediction
  const data::DataPoint &operator[](const std::string &name) const;

  /// @brief Get the requested prediction
  /// @param name The name of the prediction
  /// @return The requested prediction
  data::DataPoint &operator[](const std::string &name);

protected:
protected:
  /// @brief The timestamp of the starting point.
  DECLARE_PROTECTED_MEMBER(std::chrono::system_clock::time_point, StartingTime);

  /// @brief The predictions made by the analyzers
  std::map<std::string, data::DataPoint> m_Predictions;
  std::map<std::string, data::DataPoint> getPredictions() const {
    return m_Predictions;
  }
};

} // namespace NEUROBIO_NAMESPACE::analyzer
#endif // __NEUROBIO_ANALYZER_PREDICTIONS_H__