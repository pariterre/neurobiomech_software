#include "Analyzer/EventPrediction.h"
#include "nlohmann/json.hpp"

using namespace NEUROBIO_NAMESPACE::analyzer;

EventPrediction::EventPrediction(const std::vector<double> &values,
                                 size_t currentPhase, bool hasPhaseIncremented)
    : m_CurrentPhase(currentPhase), m_HasPhaseIncremented(hasPhaseIncremented),
      Prediction(values) {};

nlohmann::json EventPrediction::serialize() const {
  nlohmann::json json = Prediction::serialize();
  json["phase"] = m_CurrentPhase;
  json["isNewPhase"] = m_HasPhaseIncremented;
  return json;
}