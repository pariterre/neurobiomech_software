#include "Analyzer/Predictions.h"

#include "Data/DataPoint.h"

using namespace NEUROBIO_NAMESPACE::data;
using namespace NEUROBIO_NAMESPACE::analyzer;

std::map<std::string, DataPoint> parseJson(const nlohmann::json &json) {
  std::map<std::string, DataPoint> predictions;
  for (const auto &[name, value] : json.items()) {
    predictions[name] = DataPoint(value);
  }
  return predictions;
}

Predictions::Predictions() : m_StartingTime(std::chrono::system_clock::now()) {}

Predictions::Predictions(const nlohmann::json &json)
    : m_StartingTime(
          std::chrono::microseconds(json["starting_time"].get<int64_t>())),
      m_Predictions(parseJson(json["data"])) {}

void Predictions::add(const std::string &name) {
  m_Predictions[name] = DataPoint();
}

void Predictions::set(const std::string &name, const DataPoint &value) {
  m_Predictions[name] = value;
}

void Predictions::remove(const std::string &analyzerName) {
  m_Predictions.erase(analyzerName);
}

size_t Predictions::size() const { return m_Predictions.size(); }

void Predictions::reset() {
  m_StartingTime = std::chrono::system_clock::now();
  m_Predictions.clear();
}

nlohmann::json Predictions::serialize() const {
  nlohmann::json json = nlohmann::json::object();
  json["starting_time"] = std::chrono::duration_cast<std::chrono::microseconds>(
                              m_StartingTime.time_since_epoch())
                              .count();
  json["data"] = nlohmann::json::object();
  for (const auto &[name, prediction] : m_Predictions) {
    json["data"][name] = prediction.serialize();
  }
  return json;
}

const DataPoint &Predictions::operator[](const std::string &name) const {
  return m_Predictions.at(name);
}

DataPoint &Predictions::operator[](const std::string &name) {
  return m_Predictions.at(name);
}