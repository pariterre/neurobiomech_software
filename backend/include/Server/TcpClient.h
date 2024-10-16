#ifndef __STIMWALKER_SERVER_TCP_CLIENT_H__
#define __STIMWALKER_SERVER_TCP_CLIENT_H__

#include "stimwalkerConfig.h"

#include "Data/TimeSeries.h"
#include "Server/TcpServer.h"
#include "Utils/CppMacros.h"
#include <asio.hpp>

namespace STIMWALKER_NAMESPACE::server {

class TcpClient {
public:
  TcpClient(std::string host = "localhost", int commandPort = 5000,
            int dataPort = 5001);
  ~TcpClient();
  TcpClient(const TcpClient &) = delete;

  bool connect();
  bool disconnect();

  bool addDelsysDevice();
  bool addMagstimDevice();

  bool removeDelsysDevice();
  bool removeMagstimDevice();

  bool startRecording();
  bool stopRecording();

  bool updateData();

protected:
  DECLARE_PROTECTED_MEMBER(data::TimeSeries, Data);

protected:
  DECLARE_PROTECTED_MEMBER(std::string, Host);
  DECLARE_PROTECTED_MEMBER(int, CommandPort);
  DECLARE_PROTECTED_MEMBER(int, DataPort);

  DECLARE_PROTECTED_MEMBER(bool, IsConnected);

  bool sendCommandWithConfirmation(TcpServerCommand command);
  TcpServerResponse waitForResponse();

  std::array<char, 8> constructCommandPacket(TcpServerCommand command);
  TcpServerResponse parseResponsePacket(const std::array<char, 8> &buffer);

private:
  DECLARE_PRIVATE_MEMBER_NOGET(asio::io_context, Context);
  DECLARE_PRIVATE_MEMBER_NOGET(std::unique_ptr<asio::ip::tcp::socket>,
                               CommandSocket);
  DECLARE_PRIVATE_MEMBER_NOGET(std::unique_ptr<asio::ip::tcp::socket>,
                               DataSocket);

  DECLARE_PRIVATE_MEMBER_NOGET(std::uint32_t, ProtocolVersion)
};

} // namespace STIMWALKER_NAMESPACE::server

#endif // __STIMWALKER_SERVER_TCP_SERVER_H__