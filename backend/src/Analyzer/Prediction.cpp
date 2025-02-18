#include "Analyzer/Prediction.h"

#include "Analyzer/EventPrediction.h"

using namespace NEUROBIO_NAMESPACE::analyzer;

Prediction::Prediction()
    : m_TimeStamp(std::chrono::microseconds(0)), m_Data({}) {}

Prediction::Prediction(const std::chrono::microseconds &timeStamp,
                       const std::vector<double> &data)
    : m_Data(data) {};

Prediction::Prediction(const nlohmann::json &json)
    : m_TimeStamp(json["time_stamp"].get<int64_t>()),
      m_Data(json["data"].get<std::vector<double>>()) {};

size_t Prediction::size() const { return m_Data.size(); }

double Prediction::operator[](size_t index) const { return m_Data.at(index); }

nlohmann::json Prediction::serialize() const {
  return nlohmann::json({{"type", getPredictionType()},
                         {"time_stamp", m_TimeStamp.count()},
                         {"data", m_Data}});
}

std::unique_ptr<Prediction>
Prediction::deserialize(const nlohmann::json &json) {
  auto type = json["type"].get<PredictionType>();

  switch (type) {
  case PredictionType::ANALOG:
    return std::make_unique<Prediction>(json);
  case PredictionType::ANALOG_WITH_EVENTS:
    return std::make_unique<EventPrediction>(json);
  default:
    throw std::invalid_argument("Unknown prediction type");
  }
}

PredictionType Prediction::getPredictionType() const {
  return PredictionType::ANALOG;
}