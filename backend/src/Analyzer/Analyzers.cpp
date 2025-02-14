#include "Analyzer/Analyzers.h"

#include "Analyzer/LiveAnalyzer.h"
#include "Analyzer/Prediction.h"
#include "Analyzer/WalkingCycleFromDelsysPressureAnalyzer.h"
#include "Utils/Logger.h"

using namespace NEUROBIO_NAMESPACE::analyzer;

std::map<size_t, std::unique_ptr<Prediction>>
Analyzers::predict(const std::map<size_t, data::TimeSeries> &data) const {
  std::map<size_t, std::unique_ptr<Prediction>> predictions;
  for (const auto &analyzer : m_Analyzers) {
    predictions[analyzer.first] = analyzer.second->predict(data);
  }
  return predictions;
}

size_t Analyzers::add(std::unique_ptr<LiveAnalyzer> analyzer) {
  static size_t analyzerId = 0;
  m_Analyzers[analyzerId] = std::move(analyzer);
  return analyzerId++;
}

size_t Analyzers::add(const nlohmann::json &json) {
  // Get the type of the analyzer to create
  AvailableAnalyzers type = json.at("type").get<AvailableAnalyzers>();

  // Create the analyzer
  std::unique_ptr<LiveAnalyzer> analyzer;
  switch (type) {
  case WALKING_CYCLE_FROM_DELSYS_PRESSURE_ANALYZER:
    analyzer = std::make_unique<WalkingCycleFromDelsysPressureAnalyzer>(
        json.at("delsysDeviceIndex"), json.at("channelIndex"),
        json.at("heelStrikeThreshold"), json.at("toeOffThreshold"),
        json.at("learningRate"));
    break;
  default:
    utils::Logger::getInstance().fatal("Unknown analyzer type");
  }

  return add(std::move(analyzer));
}

void Analyzers::remove(size_t analyzerId) { m_Analyzers.erase(analyzerId); }

std::vector<size_t> Analyzers::getAnalyzerIds() const {
  std::vector<size_t> analyzerIds;
  for (const auto &analyzer : m_Analyzers) {
    analyzerIds.push_back(analyzer.first);
  }
  return analyzerIds;
}

size_t Analyzers::size() const { return m_Analyzers.size(); }

void Analyzers::clear() { m_Analyzers.clear(); }

const LiveAnalyzer &Analyzers::operator[](size_t analyzerId) const {
  try {
    return *m_Analyzers.at(analyzerId);
  } catch (const std::out_of_range &) {
    std::string message =
        "Analyzer with id " + std::to_string(analyzerId) + " does not exist";
    utils::Logger::getInstance().fatal(message);
    throw std::out_of_range(message);
  }
}

LiveAnalyzer &Analyzers::getAnalyzer(size_t analyzerId) {
  try {
    return *m_Analyzers.at(analyzerId);
  } catch (const std::out_of_range &) {
    std::string message =
        "Analyzer with id " + std::to_string(analyzerId) + " does not exist";
    utils::Logger::getInstance().fatal(message);
    throw std::out_of_range(message);
  }
}
