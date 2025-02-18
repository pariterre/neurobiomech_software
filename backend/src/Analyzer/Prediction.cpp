#include "Analyzer/Prediction.h"
#include "nlohmann/json.hpp"

#include "Analyzer/EventPrediction.h"

using namespace NEUROBIO_NAMESPACE::analyzer;

Prediction::Prediction(const std::vector<double> &values) : m_Values(values) {};

Prediction::Prediction(const nlohmann::json &json)
    : m_Values(json["values"].get<std::vector<double>>()) {};

nlohmann::json Prediction::serialize() const {
  nlohmann::json json;
  json["type"] = getPredictionType();
  json["values"] = m_Values;
  return json;
}

std::unique_ptr<Prediction>
Prediction::deserialize(const nlohmann::json &json) {
  if (json["type"] == "Prediction") {
    return std::make_unique<Prediction>(json);
  } else if (json["type"] == "EventPrediction") {
    return std::make_unique<EventPrediction>(json);
  } else {
    throw std::invalid_argument("Unknown prediction type");
  }
}

std::string Prediction::getPredictionType() const { return "Prediction"; }