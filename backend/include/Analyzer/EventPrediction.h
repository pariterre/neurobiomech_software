#ifndef __NEUROBIO_ANALYZER_EVENT_PREDICTION_H__
#define __NEUROBIO_ANALYZER_EVENT_PREDICTION_H__

#include "Analyzer/Prediction.h"

namespace NEUROBIO_NAMESPACE::analyzer {

/// @brief Class to store a prediction
class EventPrediction : public Prediction {

public:
  /// @brief Constructor of the EventPrediction
  EventPrediction(const std::vector<double> &values, size_t currentPhase,
                  bool hasPhaseIncremented);

  /// @brief Constructor of the EventPrediction from a json object
  /// @param json The json object to deserialize
  EventPrediction(const nlohmann::json &json);

public:
  /// @brief Destructor of the EventPrediction
  virtual ~EventPrediction() = default;

  /// @brief Get a serialized version of the prediction
  /// @return The serialized version of the prediction
  nlohmann::json serialize() const override;

  /// @brief Get the type of the prediction
  /// @return The type of the prediction
  PredictionType getPredictionType() const override;

protected:
  /// @brief Holds the index of the current phase
  DECLARE_PROTECTED_MEMBER(size_t, CurrentPhase);

  /// @brief If this event prediction had its phase incremented
  DECLARE_PROTECTED_MEMBER(bool, HasPhaseIncremented);
};

} // namespace NEUROBIO_NAMESPACE::analyzer
#endif // __NEUROBIO_ANALYZER_EVENT_PREDICTION_H__