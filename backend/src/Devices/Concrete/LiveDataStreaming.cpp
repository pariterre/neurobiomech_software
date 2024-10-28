#include "Devices/Concrete/LiveDataStreaming.h"

#include "Utils/Logger.h"

using namespace STIMWALKER_NAMESPACE::devices;

LiveDataStreaming::LiveDataStreaming(std::string clientIp, int liveDataPort)
    : m_LiveDataCheckIntervals(std::chrono::milliseconds(100)),
      m_ClientIp(clientIp), m_LiveDataPort(liveDataPort),
      m_LiveDataSocket(std::make_unique<asio::ip::udp::socket>(
          m_LiveDataContext,
          asio::ip::udp::endpoint(asio::ip::udp::v4(), m_LiveDataPort))) {

  m_LiveDataSocket->connect(asio::ip::udp::endpoint(
      asio::ip::address::from_string(m_ClientIp), m_LiveDataPort));
}

LiveDataStreaming::~LiveDataStreaming() { m_LiveDataSocket->close(); }

void LiveDataStreaming::startKeepLiveDataWorkerAlive() {
  m_KeepLiveDataWorkerAliveTimer = std::make_unique<asio::steady_timer>(
      m_LiveDataContext, m_LiveDataCheckIntervals);
  keepLiveDataWorkerAlive(m_LiveDataCheckIntervals);
}

void LiveDataStreaming::keepLiveDataWorkerAlive(
    std::chrono::microseconds timeout) {

  // Set a timer that will call [pingDataWorker] every [timeout] milliseconds
  m_KeepLiveDataWorkerAliveTimer->expires_after(timeout);

  m_KeepLiveDataWorkerAliveTimer->async_wait([this](const auto &errorCode) {
    // Get the current time
    auto now = std::chrono::high_resolution_clock::now();

    // If errorCode is not false, it means the timer was stopped by the user, or
    // the device was disconnected. In both cases, do nothing and return
    if (!m_LiveDataSocket->is_open() || !errorCode) {
      m_KeepLiveDataWorkerAliveTimer->cancel();
      return;
    }

    // Otherwise, send a [dataCheck] command to allow the user to check for
    // new data
    sendLiveDataCallback();

    // Once it's done, repeat the process, but take into account the time it
    // took to execute the [dataCheck] method
    auto timeToExecute = std::chrono::high_resolution_clock::now() - now;
    auto next = m_LiveDataCheckIntervals - timeToExecute;

    keepLiveDataWorkerAlive(
        std::chrono::duration_cast<std::chrono::microseconds>(next));
  });
}

void LiveDataStreaming::sendLiveDataCallback() {
  // TODO Change this for an actual callback
  auto &logger = utils::Logger::getInstance();
  logger.info("Sending live data callback");
}