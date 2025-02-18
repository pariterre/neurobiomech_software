#ifndef __NEUROBIO_ANALYZER_PREDICTION_H__
#define __NEUROBIO_ANALYZER_PREDICTION_H__

#include "Utils/CppMacros.h"
#include "neurobioConfig.h"
#include "nlohmann/json.hpp"
#include <map>
#include <memory>
#include <vector>

namespace NEUROBIO_NAMESPACE::analyzer {

class Prediction {
public:
  /// @brief Constructor of the Prediction
  /// @param values The values of the prediction
  Prediction(const std::vector<double> &values);

  /// @brief Constructor of the Prediction from a json object
  /// @param json The json object to deserialize
  Prediction(const nlohmann::json &json);

public:
  /// @brief Destructor of the Prediction
  virtual ~Prediction() = default;

  /// @brief Get a serialized version of the prediction
  /// @return The serialized version of the prediction
  virtual nlohmann::json serialize() const;

  /// @brief Deserialize the prediction from a json object
  /// @param json The json object to deserialize
  /// @return The deserialized prediction
  static std::unique_ptr<Prediction> deserialize(const nlohmann::json &json);

  /// @brief Get the type of the prediction
  /// @return The type of the prediction
  virtual std::string getPredictionType() const;

protected:
  /// @brief Holds the prediction
  DECLARE_PROTECTED_MEMBER(std::vector<double>, Values);
};

} // namespace NEUROBIO_NAMESPACE::analyzer
#endif // __NEUROBIO_ANALYZER_PREDICTION_H__