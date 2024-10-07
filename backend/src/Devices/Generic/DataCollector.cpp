#include "Devices/Generic/DataCollector.h"

#include "Devices/Generic/Exceptions.h"
#include "Utils/Logger.h"

using namespace STIMWALKER_NAMESPACE::devices;

void DataCollector::startRecording() {
  auto &logger = utils::Logger::getInstance();

  if (m_IsRecording) {
    logger.warning("The data collector " + dataCollectorName() +
                   " is already recording");
    return;
  }

  handleStartRecording();

  m_TimeSeries = data::TimeSeries();
  m_IsRecording = true;
  logger.info("The data collector " + dataCollectorName() +
              " is now recording");
}

void DataCollector::stopRecording() {
  auto &logger = utils::Logger::getInstance();

  if (!m_IsRecording) {
    logger.warning("The data collector " + dataCollectorName() +
                   " is not recording");
  }

  handleStopRecording();

  m_IsRecording = false;
  logger.info("The data collector " + dataCollectorName() +
              " has stopped recording");
}

const data::TimeSeries &DataCollector::getTrialData() const {
  return m_TimeSeries;
}

void DataCollector::addDataPoint(const data::DataPoint &dataPoint) {
  m_TimeSeries.add(dataPoint);
  onNewData.notifyListeners(dataPoint);
}

void DataCollector::addDataPoints(const std::vector<data::DataPoint> &data) {
  for (auto d : data) {
    m_TimeSeries.add(d);
  }
  onNewData.notifyListeners(data.back());
}