#include "Analyzer/EventPrediction.h"
#include "nlohmann/json.hpp"

using namespace NEUROBIO_NAMESPACE::analyzer;

EventPrediction::EventPrediction(const std::vector<double> &values,
                                 size_t currentPhase, bool hasPhaseIncremented)
    : m_CurrentPhase(currentPhase), m_HasPhaseIncremented(hasPhaseIncremented),
      Prediction(values) {};

EventPrediction::EventPrediction(const nlohmann::json &json)
    : m_CurrentPhase(json["phase"].get<size_t>()),
      m_HasPhaseIncremented(json["isNewPhase"].get<bool>()),
      Prediction(json["values"].get<std::vector<double>>()) {};

nlohmann::json EventPrediction::serialize() const {
  nlohmann::json json = Prediction::serialize();
  json["phase"] = m_CurrentPhase;
  json["isNewPhase"] = m_HasPhaseIncremented;
  return json;
}

std::string EventPrediction::getPredictionType() const {
  return "EventPrediction";
}
