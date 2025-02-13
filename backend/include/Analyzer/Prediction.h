#ifndef __NEUROBIO_ANALYZER_PREDICTION_H__
#define __NEUROBIO_ANALYZER_PREDICTION_H__

#include "Utils/CppMacros.h"
#include "neurobioConfig.h"
#include "nlohmann/json.hpp"
#include <map>
#include <memory>
#include <vector>

namespace NEUROBIO_NAMESPACE::analyzer {

class Prediction {
public:
  /// @brief Constructor of the Prediction
  Prediction(const std::vector<double> &values);

public:
  /// @brief Destructor of the Prediction
  virtual ~Prediction() = default;

  /// @brief Get a serialized version of the prediction
  /// @return The serialized version of the prediction
  virtual nlohmann::json serialize() const;

protected:
  /// @brief Holds the prediction
  DECLARE_PROTECTED_MEMBER(std::vector<double>, Values);
};

} // namespace NEUROBIO_NAMESPACE::analyzer
#endif // __NEUROBIO_ANALYZER_PREDICTION_H__