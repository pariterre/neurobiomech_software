#ifndef __STIMWALKER_SERVER_TCP_SERVER_H__
#define __STIMWALKER_SERVER_TCP_SERVER_H__

#include "stimwalkerConfig.h"

#include "Devices/Devices.h"
#include "Utils/CppMacros.h"
#include <asio.hpp>
#include <mutex>

namespace STIMWALKER_NAMESPACE::server {

enum TcpServerStatus { INITIALIZING, CONNECTED };

enum class TcpServerCommand : std::uint32_t {
  HANDSHAKE,
  CONNECT_DELSYS,
  CONNECT_MAGSTIM,
  DISCONNECT_DELSYS,
  DISCONNECT_MAGSTIM,
  START_RECORDING,
  STOP_RECORDING,
  PAUSE_RECORDING,
  RESUME_RECORDING,
  ERROR,
};

enum class TcpServerResponse : std::uint32_t { OK, NOK };

class TcpServer {
public:
  TcpServer(int port = 5000);
  ~TcpServer();
  TcpServer(const TcpServer &) = delete;

  void startServer();
  void disconnectClient();
  void stopServer();

protected:
  DECLARE_PROTECTED_MEMBER(devices::Devices, Devices);

  DECLARE_PROTECTED_MEMBER(int, Port);

  DECLARE_PROTECTED_MEMBER(bool, IsStarted);
  DECLARE_PROTECTED_MEMBER(bool, IsClientConnected);

  bool handleHandshake(TcpServerCommand command);
  DECLARE_PROTECTED_MEMBER_NOGET(std::unique_ptr<asio::ip::tcp::socket>,
                                 Socket);

  std::array<char, 8> constructResponsePacket(TcpServerResponse response);
  TcpServerCommand parsePacket(const std::array<char, 8> &buffer);

private:
  DECLARE_PRIVATE_MEMBER_NOGET(asio::io_context, Context);
  DECLARE_PRIVATE_MEMBER_NOGET(std::mutex, Mutex);

  DECLARE_PRIVATE_MEMBER_NOGET(std::unique_ptr<asio::ip::tcp::acceptor>,
                               Acceptor);

  DECLARE_PRIVATE_MEMBER_NOGET(TcpServerStatus, Status);

  DECLARE_PRIVATE_MEMBER_NOGET(std::uint32_t, CurrentVersion)
};

} // namespace STIMWALKER_NAMESPACE::server

#endif // __STIMWALKER_SERVER_TCP_SERVER_H__