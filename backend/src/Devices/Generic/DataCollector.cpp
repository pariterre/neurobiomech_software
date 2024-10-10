#include "Devices/Generic/DataCollector.h"

#include "Devices/Exceptions.h"
#include "Utils/Logger.h"

using namespace STIMWALKER_NAMESPACE::data;
using namespace STIMWALKER_NAMESPACE::devices;

DataCollector::DataCollector(size_t channelCount,
                             std::unique_ptr<data::TimeSeries> timeSeries)
    : m_DataChannelCount(channelCount), m_IsRecording(false),
      m_TimeSeries(std::move(timeSeries)) {}

void DataCollector::startRecording() {
  auto &logger = utils::Logger::getInstance();

  if (m_IsRecording) {
    logger.warning("The data collector " + dataCollectorName() +
                   " is already recording");
    return;
  }

  m_IsRecording = handleStartRecording();
  m_HasFailedToStartRecording = !m_IsRecording;
  m_TimeSeries->clear();

  if (m_IsRecording) {
    logger.info("The data collector " + dataCollectorName() +
                " is now recording");
  } else {
    logger.fatal("The data collector " + dataCollectorName() +
                 " failed to start recording");
  }
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

const TimeSeries &DataCollector::getTrialData() const { return *m_TimeSeries; }

void DataCollector::addDataPoint(DataPoint &dataPoint) {
  if (!m_IsRecording) {
    return;
  }

  m_TimeSeries->add(dataPoint);
  onNewData.notifyListeners(dataPoint);
}

void DataCollector::addDataPoints(std::vector<DataPoint> &data) {
  if (!m_IsRecording) {
    return;
  }

  for (auto d : data) {
    m_TimeSeries->add(d);
  }
  onNewData.notifyListeners(data.back());
}