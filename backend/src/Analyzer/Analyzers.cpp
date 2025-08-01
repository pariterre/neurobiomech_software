#include "Analyzer/Analyzers.h"

#include "Analyzer/Analyzer.h"
#include "Analyzer/CyclicTimedEventsAnalyzer.h"
#include "Analyzer/EventConditions.h"
#include "Data/DataPoint.h"
#include "Utils/Logger.h"

using namespace NEUROBIO_NAMESPACE::data;
using namespace NEUROBIO_NAMESPACE::analyzer;

Predictions
Analyzers::predict(const std::map<std::string, data::TimeSeries> &data) {
  std::shared_lock lock(m_MutexAnalyzers);
  for (const auto &analyzer : m_Analyzers) {
    m_LastPredictions[analyzer.second->getName()] =
        analyzer.second->predict(data);
  }
  return m_LastPredictions;
}

size_t Analyzers::getAnalyzerId(const std::string &analyzerName) const {
  std::shared_lock lock(const_cast<std::shared_mutex &>(m_MutexAnalyzers));
  for (const auto &analyzer : m_Analyzers) {
    if (analyzer.second->getName() == analyzerName) {
      return analyzer.first;
    }
  }
  throw std::invalid_argument("Analyzer with name " + analyzerName +
                              " does not exist");
}

size_t Analyzers::add(std::unique_ptr<Analyzer> analyzer) {
  std::unique_lock lock(m_MutexAnalyzers);

  static size_t uniqueId = 1;
  m_Analyzers[uniqueId] = std::move(analyzer);
  m_Analyzers[uniqueId]->setReferenceTime(m_LastPredictions.getStartingTime());
  m_LastPredictions.add(m_Analyzers[uniqueId]->getName());
  return uniqueId++;
}

size_t Analyzers::add(const nlohmann::json &json) {
  auto &logger = utils::Logger::getInstance();

  // Create the analyzer
  try {
    // Get the type of the analyzer to create
    std::string analyzerType = json.at("analyzer_type");

    if (analyzerType == CyclicTimedEventsAnalyzer::getSerializedName()) {
      logger.info("Creating a cyclic timed events analyzer (" +
                  json.at("name").get<std::string>() + ") from analogs");
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

void Analyzers::remove(const std::string &analyzerName) {
  remove(getAnalyzerId(analyzerName));
}

void Analyzers::remove(size_t analyzerId) {
  std::unique_lock lock(m_MutexAnalyzers);
  utils::Logger::getInstance().info("Removing analyzer with id " +
                                    std::to_string(analyzerId) + " (" +
                                    m_Analyzers[analyzerId]->getName() + ")");
  m_LastPredictions.remove(m_Analyzers[analyzerId]->getName());
  m_Analyzers.erase(analyzerId);
}

std::vector<size_t> Analyzers::getAnalyzerIds() const {
  std::shared_lock lock(const_cast<std::shared_mutex &>(m_MutexAnalyzers));
  std::vector<size_t> analyzerIds;
  for (const auto &analyzer : m_Analyzers) {
    analyzerIds.push_back(analyzer.first);
  }
  return analyzerIds;
}

size_t Analyzers::size() const { return m_Analyzers.size(); }

void Analyzers::clear() {
  std::unique_lock lock(m_MutexAnalyzers);
  m_Analyzers.clear();
  m_LastPredictions.reset();
}

const Analyzer &Analyzers::operator[](size_t analyzerId) const {
  std::shared_lock lock(const_cast<std::shared_mutex &>(m_MutexAnalyzers));
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
  std::shared_lock lock(m_MutexAnalyzers);
  try {
    return *m_Analyzers.at(analyzerId);
  } catch (const std::out_of_range &) {
    std::string message =
        "Analyzer with id " + std::to_string(analyzerId) + " does not exist";
    utils::Logger::getInstance().fatal(message);
    throw std::out_of_range(message);
  }
}

nlohmann::json Analyzers::getSerializedConfigurations() const {
  std::shared_lock lock(const_cast<std::shared_mutex &>(m_MutexAnalyzers));
  nlohmann::json config;
  for (const auto &analyzer : m_Analyzers) {
    config.push_back(analyzer.second->getSerializedConfiguration());
  }
  return config;
}