#ifndef __NEUROBIO_ANALYZER_PREDICTION_H__
#define __NEUROBIO_ANALYZER_PREDICTION_H__

#include "neurobioConfig.h"

#include "Utils/CppMacros.h"
#include <map>
#include <nlohmann/json.hpp>

namespace NEUROBIO_NAMESPACE::analyzer {

enum class PredictionType {
  ANALOG,
  ANALOG_WITH_EVENTS,
};

/// @brief Class to store a prediction
class Prediction {

public:
  /// @brief Empty constructor
  Prediction();

  /// @brief Constructor of the Prediction
  /// @param timeStamp The time stamp (i.e. elapsed time since starting time)
  /// @param data The data to store
  Prediction(const std::chrono::microseconds &timeStamp,
             const std::vector<double> &data);

  /// @brief Constructor of the Prediction from a json object
  /// @param json The json object to deserialize
  Prediction(const nlohmann::json &json);

  /// @brief Destructor of the Prediction
  virtual ~Prediction() = default;

  /// @brief Get the number of predictions
  /// @return The number of predictions
  size_t size() const;

  /// @brief Get the specific data
  /// @param index The index of the data
  /// @return The data at the given index
  double operator[](size_t index) const;

  /// @brief Get a serialized version of the prediction
  /// @return The serialized version of the prediction
  virtual nlohmann::json serialize() const;

  /// @brief Deserialize the prediction from a json object
  /// @param json The json object to deserialize
  /// @return The deserialized prediction
  static std::unique_ptr<Prediction> deserialize(const nlohmann::json &json);

protected:
  /// @brief Get the type of the prediction
  /// @return The type of the prediction
  virtual PredictionType getPredictionType() const;

  /// @brief The time stamp of the data
  DECLARE_PROTECTED_MEMBER(std::chrono::microseconds, TimeStamp);

  /// @brief Holds the prediction
  DECLARE_PROTECTED_MEMBER(std::vector<double>, Data);
};

} // namespace NEUROBIO_NAMESPACE::analyzer
#endif // __NEUROBIO_ANALYZER_PREDICTION_H__