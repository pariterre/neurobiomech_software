#include "Analyzer/Prediction.h"
#include "nlohmann/json.hpp"

using namespace NEUROBIO_NAMESPACE::analyzer;

Prediction::Prediction(const std::vector<double> &values) : m_Values(values) {};

nlohmann::json Prediction::serialize() const {
  nlohmann::json json;
  json["values"] = m_Values;
  return json;
}