#include "Devices/Generic/DataCollector.h"

#include "Devices/Exceptions.h"
#include "Utils/Logger.h"

using namespace STIMWALKER_NAMESPACE::devices;

void DataCollector::startRecording() {
  auto &logger = utils::Logger::getInstance();

  if (m_IsRecording) {
    logger.warning("The data collector " + dataCollectorName() +
                   " is already recording");
    return;
  }

  if (!handleStartRecording()) {
    logger.fatal("The data collector " + dataCollectorName() +
                 " failed to start recording");
    return;
  }

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
    return;
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
  if (!m_IsRecording) {
    return;
  }

  m_TimeSeries.add(dataPoint);
  onNewData.notifyListeners(dataPoint);
}

void DataCollector::addDataPoints(const std::vector<data::DataPoint> &data) {
  if (!m_IsRecording) {
    return;
  }

  for (auto d : data) {
    m_TimeSeries.add(d);
  }
  onNewData.notifyListeners(data.back());
}