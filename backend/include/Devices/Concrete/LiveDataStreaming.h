#ifndef __STIMWALKER_DEVICES_LIVE_DATA_STREAMING_H__
#define __STIMWALKER_DEVICES_LIVE_DATA_STREAMING_H__

#include "stimwalkerConfig.h"

#include "Utils/CppMacros.h"
#include <asio.hpp>
#include <map>
#include <memory>
#include <mutex>
#include <thread>

namespace STIMWALKER_NAMESPACE::devices {

/// @brief Abstract class for devices
class LiveDataStreaming {

public:
  /// @brief Constructor
  /// @param clientIp The IP of the client
  /// @param liveDataPort The port to communicate the live data
  LiveDataStreaming(std::string clientIp, int liveDataPort);

  // Delete copy constructor and assignment operator, this class cannot be
  // copied because of the mutex member
  LiveDataStreaming(const LiveDataStreaming &) = delete;
  LiveDataStreaming &operator=(const LiveDataStreaming &) = delete;

  ~LiveDataStreaming();

protected:
  /// @brief Streaming intervals
  DECLARE_PROTECTED_MEMBER(std::chrono::microseconds, LiveDataCheckIntervals);

  /// @brief The client ip address
  DECLARE_PROTECTED_MEMBER(std::string, ClientIp);

  /// @brief The port to listen to communicate the live data
  DECLARE_PROTECTED_MEMBER(int, LiveDataPort);

  /// @brief The io context of the connexion
  DECLARE_PROTECTED_MEMBER_NOGET(asio::io_context, LiveDataContext);

  /// @brief The socket that is connected to the client for live data
  DECLARE_PROTECTED_MEMBER_NOGET(std::unique_ptr<asio::ip::udp::socket>,
                                 LiveDataSocket);

    void sendLiveDataCallback();

private:
  /// @brief Get if the device is currently streaming data
  /// @return True if the device is streaming data, false otherwise
  DECLARE_PROTECTED_MEMBER(bool, IsStreamingData)

  /// @brief Start the keep-alive mechanism
  virtual void startKeepLiveDataWorkerAlive();

  /// @brief Set a worker thread to keep the device alive
  /// @param timeout The time to wait before sending the next keep-alive
  /// command. This usually is the [KeepWorkerAliveInterval] value, but can be
  /// overridden
  virtual void keepLiveDataWorkerAlive(std::chrono::microseconds timeout);

  /// @brief The worker timer
  DECLARE_PROTECTED_MEMBER_NOGET(std::unique_ptr<asio::steady_timer>,
                                 KeepLiveDataWorkerAliveTimer)
};

} // namespace STIMWALKER_NAMESPACE::devices

#endif // __STIMWALKER_DEVICES_LIVE_DATA_STREAMING_H__