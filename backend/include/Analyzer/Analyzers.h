#ifndef __NEUROBIO_ANALYZER_ANALYZERS_H__
#define __NEUROBIO_ANALYZER_ANALYZERS_H__

#include "neurobioConfig.h"

#include "Analyzer/Predictions.h"
#include <mutex>
#include <shared_mutex>

namespace NEUROBIO_NAMESPACE::data {
class TimeSeries;
} // namespace NEUROBIO_NAMESPACE::data

namespace NEUROBIO_NAMESPACE::analyzer {
class Analyzer;

class Analyzers {
public:
  /// @brief Constructor of the Analyzers
  Analyzers() = default;

  /// @brief Destructor of the Analyzers
  ~Analyzers() = default;

public:
  /// @brief Predict using all the analyzers
  /// @param data The data to predict
  /// @return The predictions
  Predictions predict(const std::map<std::string, data::TimeSeries> &data);

  /// @brief Get an analyzer id from its name
  /// @param analyzerName The name of the analyzer
  /// @return The id of the analyzer
  size_t getAnalyzerId(const std::string &analyzerName) const;

  /// @brief Add an analyzer to the collection
  /// @param analyzer The analyzer to add
  /// @return The id of the analyzer in the collection so it can be accessed or
  /// removed later
  size_t add(std::unique_ptr<Analyzer> analyzer);

  /// @brief Add an analyzer fo the collection from a json object
  /// @param json The json object to create the analyzer from
  /// @return The id of the analyzer in the collection so it can be accessed or
  /// removed later
  size_t add(const nlohmann::json &json);

  /// @brief Remove the analyzer from the collection
  /// @param analyzerName The name of the analyzer to remove
  void remove(const std::string &analyzerName);

  /// @brief Remove the analyzer from the collection
  /// @param analyzerId The id of the analyzer (the one returned by the add
  /// method)
  void remove(size_t analyzerId);

  /// @brief Get the ids of the analyzers in the collection
  /// @return The ids of the analyzers in the collection
  std::vector<size_t> getAnalyzerIds() const;

  /// @brief Get the number of analyzers in the collection
  /// @return The number of analyzers in the collection
  size_t size() const;

  /// @brief Remove all the analyzers from the collection
  void clear();

  /// @brief Get the requested analyzer
  /// @param analyzerId The id of the analyzer (the one returned by the add
  /// method)
  /// @return The requested analyzer
  const Analyzer &operator[](size_t analyzerId) const;

  /// @brief Get the requested analyzer
  /// @param analyzerId The id of the analyzer (the one returned by the add
  /// method)
  /// @return The requested analyzer
  Analyzer &getAnalyzer(size_t analyzerId);

  /// @brief Get the configuration in a json format
  /// @return The configuration in a json format
  nlohmann::json getSerializedConfigurations() const;

protected:
  /// @brief The collection of analyzers
  std::map<size_t, std::shared_ptr<Analyzer>> m_Analyzers;

  /// @brief The predictions made by the analyzers
  Predictions m_LastPredictions;
  Predictions getLastPredictions() const { return m_LastPredictions; }

private:
  /// @brief The mutex to lock certain operations
  DECLARE_PRIVATE_MEMBER_NOGET(std::shared_mutex, MutexAnalyzers);
};

} // namespace NEUROBIO_NAMESPACE::analyzer
#endif // __NEUROBIO_ANALYZER_ANALYZERS_H__