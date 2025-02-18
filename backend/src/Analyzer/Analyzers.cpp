#include "Analyzer/Analyzers.h"

#include "Analyzer/Analyzer.h"
#include "Analyzer/CyclicTimedEventsAnalyzer.h"
#include "Analyzer/EventConditions.h"
#include "Data/DataPoint.h"
#include "Utils/Logger.h"

using namespace NEUROBIO_NAMESPACE::data;
using namespace NEUROBIO_NAMESPACE::analyzer;

std::map<std::string, DataPoint>
Analyzers::predict(const std::map<std::string, data::TimeSeries> &data) const {
  std::map<std::string, DataPoint> predictions;
  for (const auto &analyzer : m_Analyzers) {
    predictions[analyzer.second->getName()] = analyzer.second->predict(data);
  }
  return predictions;
}

size_t Analyzers::getAnalyzerId(const std::string &analyzerName) const {
  for (const auto &analyzer : m_Analyzers) {
    if (analyzer.second->getName() == analyzerName) {
      return analyzer.first;
    }
  }
  throw std::invalid_argument("Analyzer with name " + analyzerName +
                              " does not exist");
}

size_t Analyzers::add(std::unique_ptr<Analyzer> analyzer) {
  static size_t analyzerId = 1;
  m_Analyzers[analyzerId] = std::move(analyzer);
  return analyzerId++;
}

size_t Analyzers::add(const nlohmann::json &json) {
  auto &logger = utils::Logger::getInstance();

  // Get the type of the analyzer to create
  std::string analyzerType = json["analyzer_type"];

  // Create the analyzer
  try {
    if (analyzerType == "cyclic_from_analogs") {
      logger.info("Creating a cyclic timed events analyzer from analogs");
      return add(std::make_unique<CyclicTimedEventsAnalyzer>(json));
    } else {
      logger.fatal("Unknown analyzer type");
      throw std::invalid_argument("Unknown analyzer type");
    }
  } catch (const std::exception &e) {
    logger.fatal("Failed to create the analyzer: " + std::string(e.what()));
    throw e;
  }
}

void Analyzers::remove(std::string analyzerName) {
  remove(getAnalyzerId(analyzerName));
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

const Analyzer &Analyzers::operator[](size_t analyzerId) const {
  try {
    return *m_Analyzers.at(analyzerId);
  } catch (const std::out_of_range &) {
    std::string message =
        "Analyzer with id " + std::to_string(analyzerId) + " does not exist";
    utils::Logger::getInstance().fatal(message);
    throw std::out_of_range(message);
  }
}

Analyzer &Analyzers::getAnalyzer(size_t analyzerId) {
  try {
    return *m_Analyzers.at(analyzerId);
  } catch (const std::out_of_range &) {
    std::string message =
        "Analyzer with id " + std::to_string(analyzerId) + " does not exist";
    utils::Logger::getInstance().fatal(message);
    throw std::out_of_range(message);
  }
}
