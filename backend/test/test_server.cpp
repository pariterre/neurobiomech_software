#include <gtest/gtest.h>
#include <iostream>
#include <thread>

#include "Server/TcpClient.h"
#include "Server/TcpServer.h"
#include "utils.h"

static double requiredPrecision(1e-10);

#ifndef SKIP_CI_FAILING_TESTS
#ifdef WIN32
std::chrono::milliseconds failingTimeoutPeriod(500);
std::chrono::milliseconds failingBufferPeriod(100);
std::chrono::milliseconds sufficientTimeoutPeriod(4000);
#else
std::chrono::milliseconds failingTimeoutPeriod(500);
std::chrono::milliseconds failingBufferPeriod(100);
std::chrono::milliseconds sufficientTimeoutPeriod(500);
#endif
void ensureServerIsConnected(
    const NEUROBIO_NAMESPACE::server::TcpServerMock &server) {
  auto startingWaitingTime = std::chrono::high_resolution_clock::now();
  while (true) {
    if (server.isClientConnected() ||
        std::chrono::high_resolution_clock::now() >
            startingWaitingTime + sufficientTimeoutPeriod) {
      break;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}
void ensureServerIsDisconnected(
    const NEUROBIO_NAMESPACE::server::TcpServerMock &server) {
  auto startingWaitingTime = std::chrono::high_resolution_clock::now();
  while (true) {
    if (!server.isClientConnected() ||
        std::chrono::high_resolution_clock::now() >
            startingWaitingTime + sufficientTimeoutPeriod) {
      break;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

using namespace NEUROBIO_NAMESPACE;

TEST(Server, Ports) {
  auto logger = TestLogger();

  {
    server::TcpServer server;
    ASSERT_EQ(server.getCommandPort(), 5000);
    ASSERT_EQ(server.getResponsePort(), 5001);
  }

  {
    server::TcpServer server(5010, 5011);
    ASSERT_EQ(server.getCommandPort(), 5010);
    ASSERT_EQ(server.getResponsePort(), 5011);
  }
}

TEST(Server, StartServer) {
  auto logger = TestLogger();

  server::TcpServer server;
  server.startServer();
  logger.giveTimeToUpdate();

  // Test the server actually started
  ASSERT_TRUE(logger.contains("Command server started on port 5000"));
  ASSERT_TRUE(logger.contains("Response server started on port 5001"));
  logger.clear();

  // Stop the server
  server.stopServer();

  // Test the server actually stopped
  ASSERT_TRUE(logger.contains(
      "Stopping listening to ports as server is shutting down"));
  ASSERT_TRUE(logger.contains("Server has shut down"));
}

#ifndef SKIP_LONG_TESTS
TEST(Server, ClientConnexion) {
  auto logger = TestLogger();

  // Server can be shut if nothing happened
  {
    server::TcpServerMock server(5000, 5001, 5002, 5003, failingTimeoutPeriod);
    server.startServer();

    logger.giveTimeToUpdate();
    ASSERT_TRUE(logger.contains("Command server started on port 5000"));
    ASSERT_TRUE(logger.contains("Response server started on port 5001"));
    ASSERT_TRUE(logger.contains("Waiting for a new connexion"));

    server.stopServer();
    ASSERT_TRUE(logger.contains("Server has shut down"));
  }
  logger.clear();

  // Server will wait for as much as needed before someone tries to connect
  {
    server::TcpServerMock server(5000, 5001, 5002, 5003, failingTimeoutPeriod);
    server.startServer();

    std::this_thread::sleep_for(3 * failingTimeoutPeriod + failingBufferPeriod);
    logger.giveTimeToUpdate();

    asio::io_context context;
    asio::ip::tcp::resolver resolver(context);
    auto socket = std::make_unique<asio::ip::tcp::socket>(context);
    asio::connect(*socket, resolver.resolve("localhost", std::to_string(5000)));

    logger.giveTimeToUpdate();
    ASSERT_TRUE(logger.contains("Command socket connected to client, waiting "
                                "for a connexion to the response socket"));
  }
  logger.clear();

  {
    server::TcpServerMock server(5000, 5001, 5002, 5003, failingTimeoutPeriod);
    server.startServer();

    asio::io_context context;
    asio::ip::tcp::resolver resolver(context);
    auto socket = std::make_unique<asio::ip::tcp::socket>(context);
    asio::connect(*socket, resolver.resolve("localhost", std::to_string(5000)));
    std::this_thread::sleep_for(failingBufferPeriod);

    // Give some time to the message to arrive
    logger.giveTimeToUpdate();
    ASSERT_TRUE(logger.contains("Command socket connected to client, waiting "
                                "for a connexion to the response socket"));
  }
  // Server can be shut if one connexion was made and is waiting for the reponse
  // port
  ASSERT_TRUE(logger.contains("Server has shut down"));
  logger.clear();

  // Server can be shut if one connexion was made but dropped
  {
    server::TcpServerMock server(5000, 5001, 5002, 5003, failingTimeoutPeriod);
    server.startServer();

    asio::io_context context;
    asio::ip::tcp::resolver resolver(context);
    auto socket = std::make_unique<asio::ip::tcp::socket>(context);
    asio::connect(*socket, resolver.resolve("localhost", std::to_string(5000)));

    // Wait longer than the timeout
    std::this_thread::sleep_for(failingTimeoutPeriod + failingBufferPeriod);
    ASSERT_TRUE(logger.contains("Connexion to Response socket timed out (" +
                                std::to_string(failingTimeoutPeriod.count()) +
                                " ms), disconnecting client"));
  }
  ASSERT_TRUE(logger.contains("All devices are now disconnected"));
  ASSERT_TRUE(logger.contains(
      "Stopping listening to ports as server is shutting down"));
  ASSERT_TRUE(logger.contains("Server has shut down"));
  logger.clear();

  // Server can be shut if connexions to command and response ports were made,
  // but was droped before the connexion to live data
  {
    server::TcpServerMock server(5000, 5001, 5002, 5003,
                                 sufficientTimeoutPeriod);
    server.startServer();

    asio::io_context context;
    asio::ip::tcp::resolver resolver(context);
    auto commandSocket = std::make_unique<asio::ip::tcp::socket>(context);
    asio::connect(*commandSocket,
                  resolver.resolve("localhost", std::to_string(5000)));
    std::this_thread::sleep_for(failingBufferPeriod);
    auto responseSocket = std::make_unique<asio::ip::tcp::socket>(context);
    asio::connect(*responseSocket,
                  resolver.resolve("localhost", std::to_string(5001)));
    std::this_thread::sleep_for(failingBufferPeriod);

    server.setTimeoutPeriod(failingTimeoutPeriod);

    std::this_thread::sleep_for(failingTimeoutPeriod + failingBufferPeriod);
    // Give some time to the message to arrive
    logger.giveTimeToUpdate();
    ASSERT_TRUE(logger.contains("Connexion to LiveData socket timed out (" +
                                std::to_string(failingTimeoutPeriod.count()) +
                                " ms), disconnecting client"));
  }
  ASSERT_TRUE(logger.contains(
      "Stopping listening to ports as server is shutting down"));
  ASSERT_TRUE(logger.contains("Server has shut down"));
  logger.clear();

  // Server can be shut if connexions to command, response and live ports were
  // made, but was droped before the the live analysis was made
  {
    server::TcpServerMock server(5000, 5001, 5002, 5003,
                                 sufficientTimeoutPeriod);
    server.startServer();

    asio::io_context context;
    asio::ip::tcp::resolver resolver(context);
    auto commandSocket = std::make_unique<asio::ip::tcp::socket>(context);
    asio::connect(*commandSocket,
                  resolver.resolve("localhost", std::to_string(5000)));
    std::this_thread::sleep_for(failingBufferPeriod);
    auto responseSocket = std::make_unique<asio::ip::tcp::socket>(context);
    asio::connect(*responseSocket,
                  resolver.resolve("localhost", std::to_string(5001)));
    std::this_thread::sleep_for(failingBufferPeriod);
    auto liveDataSocket = std::make_unique<asio::ip::tcp::socket>(context);
    asio::connect(*liveDataSocket,
                  resolver.resolve("localhost", std::to_string(5002)));
    std::this_thread::sleep_for(failingBufferPeriod);

    server.setTimeoutPeriod(failingTimeoutPeriod);

    std::this_thread::sleep_for(failingTimeoutPeriod + failingBufferPeriod);
    // Give some time to the message to arrive
    logger.giveTimeToUpdate();
    ASSERT_TRUE(logger.contains("Connexion to LiveAnalyses socket timed out (" +
                                std::to_string(failingTimeoutPeriod.count()) +
                                " ms), disconnecting client"));
  }
  ASSERT_TRUE(logger.contains(
      "Stopping listening to ports as server is shutting down"));
  ASSERT_TRUE(logger.contains("Server has shut down"));
  logger.clear();

  // Server can be shut if connexions to command, response, live and live
  // analyses ports were made, but waiting for the handshake
  {
    server::TcpServerMock server(5000, 5001, 5002, 5003,
                                 sufficientTimeoutPeriod);
    server.startServer();

    asio::io_context context;
    asio::ip::tcp::resolver resolver(context);

    auto commandSocket = std::make_unique<asio::ip::tcp::socket>(context);
    asio::connect(*commandSocket,
                  resolver.resolve("localhost", std::to_string(5000)));
    std::this_thread::sleep_for(failingBufferPeriod);
    std::this_thread::sleep_for(failingBufferPeriod);
    auto responseSocket = std::make_unique<asio::ip::tcp::socket>(context);
    asio::connect(*responseSocket,
                  resolver.resolve("localhost", std::to_string(5001)));
    std::this_thread::sleep_for(failingBufferPeriod);
    std::this_thread::sleep_for(failingBufferPeriod);
    auto liveDataSocket = std::make_unique<asio::ip::tcp::socket>(context);
    asio::connect(*liveDataSocket,
                  resolver.resolve("localhost", std::to_string(5002)));
    std::this_thread::sleep_for(failingBufferPeriod);
    auto liveAnalysesSocket = std::make_unique<asio::ip::tcp::socket>(context);
    asio::connect(*liveAnalysesSocket,
                  resolver.resolve("localhost", std::to_string(5003)));
    std::this_thread::sleep_for(failingBufferPeriod);

    // Give some time to the message to arrive
    logger.giveTimeToUpdate();

    ASSERT_TRUE(
        logger.contains("All ports are connected, waiting for the handshake"));
  }
  ASSERT_TRUE(logger.contains("Disconnecting client"));
  ASSERT_TRUE(logger.contains("Server has shut down"));
  logger.clear();

  // Server can be shut if connexions to command, response, live data and live
  // analyses ports were made, but waiting for the handshake timed out
  {
    server::TcpServerMock server(5000, 5001, 5002, 5003,
                                 sufficientTimeoutPeriod);
    server.startServer();

    asio::io_context context;
    asio::ip::tcp::resolver resolver(context);
    auto commandSocket = std::make_unique<asio::ip::tcp::socket>(context);
    auto responseSocket = std::make_unique<asio::ip::tcp::socket>(context);
    auto liveDataSocket = std::make_unique<asio::ip::tcp::socket>(context);
    auto liveAnalysesSocket = std::make_unique<asio::ip::tcp::socket>(context);
    asio::connect(*commandSocket,
                  resolver.resolve("localhost", std::to_string(5000)));
    asio::connect(*responseSocket,
                  resolver.resolve("localhost", std::to_string(5001)));
    asio::connect(*liveDataSocket,
                  resolver.resolve("localhost", std::to_string(5002)));
    asio::connect(*liveAnalysesSocket,
                  resolver.resolve("localhost", std::to_string(5003)));

    // Wait longer than the timeout
    ensureServerIsConnected(server);
    std::this_thread::sleep_for(failingBufferPeriod);

    server.setTimeoutPeriod(failingTimeoutPeriod);
    std::this_thread::sleep_for(failingTimeoutPeriod + failingBufferPeriod);

    logger.giveTimeToUpdate();
    ASSERT_TRUE(logger.contains("Handshake timeout (" +
                                std::to_string(failingTimeoutPeriod.count()) +
                                +" ms), disconnecting client"));
    ASSERT_TRUE(logger.contains("Disconnecting client"));
  }
  ASSERT_TRUE(logger.contains("Server has shut down"));
  logger.clear();

  // Server can be shut if connexions to command, response, live data and live
  // analyses ports were made, but waiting for the handshake timed out, and a
  // connexion was retried
  {
    server::TcpServerMock server(5000, 5001, 5002, 5003,
                                 sufficientTimeoutPeriod);
    server.startServer();

    asio::io_context context;
    asio::ip::tcp::resolver resolver(context);
    auto commandSocket = std::make_unique<asio::ip::tcp::socket>(context);
    auto responseSocket = std::make_unique<asio::ip::tcp::socket>(context);
    auto liveDataSocket = std::make_unique<asio::ip::tcp::socket>(context);
    auto liveAnalysesSocket = std::make_unique<asio::ip::tcp::socket>(context);
    asio::connect(*commandSocket,
                  resolver.resolve("localhost", std::to_string(5000)));
    asio::connect(*responseSocket,
                  resolver.resolve("localhost", std::to_string(5001)));
    asio::connect(*liveDataSocket,
                  resolver.resolve("localhost", std::to_string(5002)));
    asio::connect(*liveAnalysesSocket,
                  resolver.resolve("localhost", std::to_string(5003)));

    // Wait longer than the timeout
    ensureServerIsConnected(server);
    std::this_thread::sleep_for(failingBufferPeriod);

    server.setTimeoutPeriod(failingTimeoutPeriod);
    std::this_thread::sleep_for(failingTimeoutPeriod + failingBufferPeriod);
    asio::connect(*commandSocket,
                  resolver.resolve("localhost", std::to_string(5000)));

    // Give some time to the message to arrive
    logger.giveTimeToUpdate();
    ASSERT_EQ(logger.count("Command socket connected to client, waiting for a "
                           "connexion to the response socket"),
              2);
  }
  ASSERT_TRUE(logger.contains("Server has shut down"));
  logger.clear();

  // Server can be shut if connexions to command, response, live data and live
  // analyses ports were made, but waiting for the handshake timed out, and a
  // connexion was retried but response socket connexion timed out
  {
    server::TcpServerMock server(5000, 5001, 5002, 5003,
                                 sufficientTimeoutPeriod);
    server.startServer();

    asio::io_context context;
    asio::ip::tcp::resolver resolver(context);
    auto commandSocket = std::make_unique<asio::ip::tcp::socket>(context);
    auto responseSocket = std::make_unique<asio::ip::tcp::socket>(context);
    auto liveDataSocket = std::make_unique<asio::ip::tcp::socket>(context);
    auto liveAnalysesSocket = std::make_unique<asio::ip::tcp::socket>(context);
    asio::connect(*commandSocket,
                  resolver.resolve("localhost", std::to_string(5000)));
    asio::connect(*responseSocket,
                  resolver.resolve("localhost", std::to_string(5001)));
    asio::connect(*liveDataSocket,
                  resolver.resolve("localhost", std::to_string(5002)));
    asio::connect(*liveAnalysesSocket,
                  resolver.resolve("localhost", std::to_string(5003)));

    // Wait longer than the timeout
    ensureServerIsConnected(server);
    std::this_thread::sleep_for(failingBufferPeriod);

    server.setTimeoutPeriod(failingTimeoutPeriod);
    std::this_thread::sleep_for(failingTimeoutPeriod + failingBufferPeriod);
    asio::connect(*commandSocket,
                  resolver.resolve("localhost", std::to_string(5000)));
    std::this_thread::sleep_for(failingTimeoutPeriod + failingBufferPeriod);

    ASSERT_TRUE(logger.contains("Connexion to Response socket timed out (" +
                                std::to_string(failingTimeoutPeriod.count()) +
                                " ms), "
                                "disconnecting client"));
  }
  ASSERT_TRUE(logger.contains("Server has shut down"));
  logger.clear();

  // Server can be shut if connexions to command, response, live data and live
  // analyses ports were made, but waiting for the handshake timed out, and a
  // connexion was retried and response socket connexion was made
  {
    server::TcpServerMock server(5000, 5001, 5002, 5003,
                                 sufficientTimeoutPeriod);
    server.startServer();

    asio::io_context context;
    asio::ip::tcp::resolver resolver(context);
    auto commandSocket = std::make_unique<asio::ip::tcp::socket>(context);
    auto responseSocket = std::make_unique<asio::ip::tcp::socket>(context);
    auto liveDataSocket = std::make_unique<asio::ip::tcp::socket>(context);
    auto liveAnalysesSocket = std::make_unique<asio::ip::tcp::socket>(context);
    asio::connect(*commandSocket,
                  resolver.resolve("localhost", std::to_string(5000)));
    asio::connect(*responseSocket,
                  resolver.resolve("localhost", std::to_string(5001)));
    asio::connect(*liveDataSocket,
                  resolver.resolve("localhost", std::to_string(5002)));
    asio::connect(*liveAnalysesSocket,
                  resolver.resolve("localhost", std::to_string(5003)));

    // Wait longer than the timeout
    ensureServerIsConnected(server);
    std::this_thread::sleep_for(failingBufferPeriod);

    server.setTimeoutPeriod(failingTimeoutPeriod);
    std::this_thread::sleep_for(failingTimeoutPeriod + failingBufferPeriod);
    server.setTimeoutPeriod(sufficientTimeoutPeriod);
    asio::connect(*commandSocket,
                  resolver.resolve("localhost", std::to_string(5000)));
    asio::connect(*responseSocket,
                  resolver.resolve("localhost", std::to_string(5001)));
    ensureServerIsConnected(server);

    // Give some time to the message to arrive
    logger.giveTimeToUpdate();
    ASSERT_EQ(logger.count("Response socket connected to client, waiting for a "
                           "connexion to the live data socket"),
              2);
  }
  ASSERT_TRUE(logger.contains("Server has shut down"));
  logger.clear();

  // Server can be shut if connexions to command, response, live data and live
  // analyses ports were made, but waiting for the handshake timed out, and a
  // connexion was retried and live data connexion timed out
  {
    server::TcpServerMock server(5000, 5001, 5002, 5003,
                                 sufficientTimeoutPeriod);
    server.startServer();

    asio::io_context context;
    asio::ip::tcp::resolver resolver(context);
    auto commandSocket = std::make_unique<asio::ip::tcp::socket>(context);
    auto responseSocket = std::make_unique<asio::ip::tcp::socket>(context);
    auto liveDataSocket = std::make_unique<asio::ip::tcp::socket>(context);
    auto liveAnalysesSocket = std::make_unique<asio::ip::tcp::socket>(context);
    asio::connect(*commandSocket,
                  resolver.resolve("localhost", std::to_string(5000)));
    asio::connect(*responseSocket,
                  resolver.resolve("localhost", std::to_string(5001)));
    asio::connect(*liveDataSocket,
                  resolver.resolve("localhost", std::to_string(5002)));
    asio::connect(*liveAnalysesSocket,
                  resolver.resolve("localhost", std::to_string(5003)));

    // Wait longer than the timeout
    ensureServerIsConnected(server);
    std::this_thread::sleep_for(failingBufferPeriod);

    server.setTimeoutPeriod(failingTimeoutPeriod);
    std::this_thread::sleep_for(failingTimeoutPeriod + failingBufferPeriod);
    server.setTimeoutPeriod(sufficientTimeoutPeriod);
    asio::connect(*commandSocket,
                  resolver.resolve("localhost", std::to_string(5000)));
    std::this_thread::sleep_for(failingBufferPeriod);
    asio::connect(*responseSocket,
                  resolver.resolve("localhost", std::to_string(5001)));
    std::this_thread::sleep_for(failingBufferPeriod);
    asio::connect(*liveDataSocket,
                  resolver.resolve("localhost", std::to_string(5002)));
    std::this_thread::sleep_for(failingBufferPeriod);
    ensureServerIsConnected(server);

    std::this_thread::sleep_for(failingTimeoutPeriod + failingBufferPeriod);
    // Give some time to the message to arrive
    logger.giveTimeToUpdate();
    ASSERT_TRUE(logger.contains("Connexion to LiveAnalyses socket timed out (" +
                                std::to_string(failingTimeoutPeriod.count()) +
                                " ms), disconnecting client"));
  }
  ASSERT_TRUE(logger.contains("Server has shut down"));
  logger.clear();

  // Server can be shut if connexions to command, response, live data and live
  // analyses ports were made, but waiting for the handshake timed out, and a
  // connexion was retried, but waiting for the handshake
  {
    server::TcpServerMock server(5000, 5001, 5002, 5003,
                                 sufficientTimeoutPeriod);
    server.startServer();

    asio::io_context context;
    asio::ip::tcp::resolver resolver(context);
    auto commandSocket = std::make_unique<asio::ip::tcp::socket>(context);
    auto responseSocket = std::make_unique<asio::ip::tcp::socket>(context);
    auto liveDataSocket = std::make_unique<asio::ip::tcp::socket>(context);
    auto liveAnalysesSocket = std::make_unique<asio::ip::tcp::socket>(context);
    asio::connect(*commandSocket,
                  resolver.resolve("localhost", std::to_string(5000)));
    asio::connect(*responseSocket,
                  resolver.resolve("localhost", std::to_string(5001)));
    asio::connect(*liveDataSocket,
                  resolver.resolve("localhost", std::to_string(5002)));
    asio::connect(*liveAnalysesSocket,
                  resolver.resolve("localhost", std::to_string(5003)));

    // Wait longer than the timeout
    ensureServerIsConnected(server);
    std::this_thread::sleep_for(failingBufferPeriod);

    server.setTimeoutPeriod(failingTimeoutPeriod);
    std::this_thread::sleep_for(failingTimeoutPeriod + failingBufferPeriod);
    server.setTimeoutPeriod(sufficientTimeoutPeriod);
    asio::connect(*commandSocket,
                  resolver.resolve("localhost", std::to_string(5000)));
    std::this_thread::sleep_for(failingBufferPeriod);
    asio::connect(*responseSocket,
                  resolver.resolve("localhost", std::to_string(5001)));
    std::this_thread::sleep_for(failingBufferPeriod);
    asio::connect(*liveDataSocket,
                  resolver.resolve("localhost", std::to_string(5002)));
    std::this_thread::sleep_for(failingBufferPeriod);
    asio::connect(*liveAnalysesSocket,
                  resolver.resolve("localhost", std::to_string(5003)));
    ensureServerIsConnected(server);

    std::this_thread::sleep_for(failingTimeoutPeriod + failingBufferPeriod);

    // Give some time to the message to arrive
    logger.giveTimeToUpdate();

    ASSERT_TRUE(
        logger.contains("All ports are connected, waiting for the handshake"));
  }
  ASSERT_TRUE(logger.contains("Server has shut down"));
  logger.clear();

  // Server can be shut if connexions to command, response, live data and live
  // analyses ports were made, but waiting for the handshake timed out, and a
  // connexion was retried and response socket connexion was made, but
  // handshaked timed out
  {
    server::TcpServerMock server(5000, 5001, 5002, 5003,
                                 sufficientTimeoutPeriod);
    server.startServer();

    asio::io_context context;
    asio::ip::tcp::resolver resolver(context);
    auto commandSocket = std::make_unique<asio::ip::tcp::socket>(context);
    auto responseSocket = std::make_unique<asio::ip::tcp::socket>(context);
    auto liveDataSocket = std::make_unique<asio::ip::tcp::socket>(context);
    auto liveAnalysesSocket = std::make_unique<asio::ip::tcp::socket>(context);
    asio::connect(*commandSocket,
                  resolver.resolve("localhost", std::to_string(5000)));
    asio::connect(*responseSocket,
                  resolver.resolve("localhost", std::to_string(5001)));
    asio::connect(*liveDataSocket,
                  resolver.resolve("localhost", std::to_string(5002)));
    asio::connect(*liveAnalysesSocket,
                  resolver.resolve("localhost", std::to_string(5003)));

    // Wait longer than the timeout
    ensureServerIsConnected(server);
    std::this_thread::sleep_for(failingBufferPeriod);

    server.setTimeoutPeriod(failingTimeoutPeriod);
    std::this_thread::sleep_for(failingTimeoutPeriod + failingBufferPeriod);
    server.setTimeoutPeriod(sufficientTimeoutPeriod);
    asio::connect(*commandSocket,
                  resolver.resolve("localhost", std::to_string(5000)));
    asio::connect(*responseSocket,
                  resolver.resolve("localhost", std::to_string(5001)));
    asio::connect(*liveDataSocket,
                  resolver.resolve("localhost", std::to_string(5002)));
    asio::connect(*liveAnalysesSocket,
                  resolver.resolve("localhost", std::to_string(5003)));

    ensureServerIsConnected(server);
    std::this_thread::sleep_for(failingBufferPeriod);

    server.setTimeoutPeriod(failingTimeoutPeriod);
    std::this_thread::sleep_for(failingTimeoutPeriod + failingBufferPeriod);

    ASSERT_EQ(logger.count("Handshake timeout (" +
                           std::to_string(failingTimeoutPeriod.count()) +
                           " ms), disconnecting client"),
              2);
  }
  ASSERT_TRUE(logger.contains("Server has shut down"));
  logger.clear();

  // Server can be shut if connexions was successful
  {
    server::TcpServerMock server(5000, 5001, 5002, 5003,
                                 sufficientTimeoutPeriod);
    server.startServer();

    server::TcpClient client;
    bool isConnected = client.connect();
    ASSERT_TRUE(isConnected);

    // Give some time to the message to arrive
    logger.giveTimeToUpdate();
    ASSERT_TRUE(logger.contains("Handshake from client is valid"));
  }
  ASSERT_TRUE(logger.contains("Server has shut down"));
  logger.clear();

  // Server can be shut if connexions was successful then dropped
  {
    server::TcpServerMock server(5000, 5001, 5002, 5003,
                                 sufficientTimeoutPeriod);
    server.startServer();

    server::TcpClient client;
    client.connect();
    ensureServerIsConnected(server);
    bool isDisconnected = client.disconnect();
    ASSERT_TRUE(isDisconnected);

    // Give some time to the message to arrive
    ensureServerIsDisconnected(server);
    logger.giveTimeToUpdate();
    ASSERT_TRUE(logger.contains("Disconnecting client"));
  }
  ASSERT_TRUE(logger.contains("Server has shut down"));
  logger.clear();

  // We can reconnect after disconnecting
  {
    server::TcpServerMock server(5000, 5001, 5002, 5003,
                                 sufficientTimeoutPeriod);
    server.startServer();

    server::TcpClient client;
    client.connect();
    ensureServerIsConnected(server);
    client.disconnect();
    ensureServerIsDisconnected(server);

    // Give some time
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    bool isConnected = client.connect();
    ASSERT_TRUE(isConnected);
    ensureServerIsConnected(server);

    logger.giveTimeToUpdate();
    ASSERT_EQ(logger.count("Handshake from client is valid"), 2);
  }
  ASSERT_TRUE(logger.contains("Server has shut down"));
  logger.clear();
}
#endif // SKIP_LONG_TESTS

TEST(Server, AddDevices) {
  auto logger = TestLogger();

  // Happy path
  {
    server::TcpServerMock server(5000, 5001, 5002, 5003,
                                 sufficientTimeoutPeriod);
    server.startServer();

    server::TcpClient client;
    client.connect();

    // Add the devices
    bool isDelsysEmgAdded = client.addDelsysEmgDevice();
    bool isDelsysAnalogAdded = client.addDelsysAnalogDevice();
    bool isMagstimAdded = client.addMagstimDevice();
    ASSERT_TRUE(isDelsysEmgAdded);
    ASSERT_TRUE(isDelsysAnalogAdded);
    ASSERT_TRUE(isMagstimAdded);

    // Give some time to the message to arrive
    ASSERT_TRUE(logger.contains("The device DelsysEmgDevice is now connected"));
    ASSERT_TRUE(
        logger.contains("The device DelsysAnalogDevice is now connected"));
    ASSERT_TRUE(
        logger.contains("The device MagstimRapidDevice is now connected"));
    ASSERT_TRUE(logger.contains(
        "The data collector DelsysEmgDataCollector is now streaming data"));
    ASSERT_TRUE(logger.contains(
        "The data collector DelsysAnalogDataCollector is now streaming data"));
    logger.clear();
  }
  ASSERT_TRUE(logger.contains(
      "The data collector DelsysEmgDataCollector has stopped streaming data"));
  ASSERT_TRUE(logger.contains("The data collector DelsysAnalogDataCollector "
                              "has stopped streaming data"));
  ASSERT_TRUE(
      logger.contains("The device DelsysEmgDevice is now disconnected"));
  ASSERT_TRUE(
      logger.contains("The device DelsysAnalogDevice is now disconnected"));
  ASSERT_TRUE(
      logger.contains("The device MagstimRapidDevice is now disconnected"));
  ASSERT_TRUE(logger.contains("Server has shut down"));
  logger.clear();

  // Add the same devices twice
  {
    server::TcpServerMock server(5000, 5001, 5002, 5003,
                                 sufficientTimeoutPeriod);
    server.startServer();

    server::TcpClient client;
    client.connect();

    // Add the devices
    bool isDelsysEmgAdded = client.addDelsysEmgDevice();
    bool isDelssysAnalogAdded = client.addDelsysAnalogDevice();
    bool isMagstimAdded = client.addMagstimDevice();
    ASSERT_TRUE(isDelsysEmgAdded);
    ASSERT_TRUE(isDelssysAnalogAdded);
    ASSERT_TRUE(isMagstimAdded);

    // Add the devices again
    isDelsysEmgAdded = client.addDelsysEmgDevice();
    isDelssysAnalogAdded = client.addDelsysAnalogDevice();
    isMagstimAdded = client.addMagstimDevice();
    ASSERT_FALSE(isDelsysEmgAdded);
    ASSERT_FALSE(isDelssysAnalogAdded);
    ASSERT_FALSE(isMagstimAdded);

    // Give some time to the message to arrive
    logger.giveTimeToUpdate();
    ASSERT_TRUE(logger.contains(
        "Cannot add the DelsysEmgDevice devise as it is already connected"));
    ASSERT_TRUE(logger.contains(
        "Cannot add the DelsysAnalogDevice devise as it is already connected"));
    ASSERT_TRUE(logger.contains(
        "Cannot add the MagstimRapidDevice devise as it is already connected"));
  }
  ASSERT_TRUE(logger.contains("Server has shut down"));
  logger.clear();

  // Remove devices by hand
  {
    server::TcpServerMock server(5000, 5001, 5002, 5003,
                                 sufficientTimeoutPeriod);
    server.startServer();

    server::TcpClient client;
    client.connect();

    // Add the devices
    bool isDelsysEmgAdded = client.addDelsysEmgDevice();
    bool isDelssysAnalogAdded = client.addDelsysAnalogDevice();
    bool isMagstimAdded = client.addMagstimDevice();
    ASSERT_TRUE(isDelsysEmgAdded);
    ASSERT_TRUE(isDelssysAnalogAdded);
    ASSERT_TRUE(isMagstimAdded);
    logger.clear();

    // Remove the devices
    bool isDelsysEmgRemoved = client.removeDelsysEmgDevice();
    bool isDelssysAnalogRemoved = client.removeDelsysAnalogDevice();
    bool isMagstimRemoved = client.removeMagstimDevice();
    ASSERT_TRUE(isDelsysEmgRemoved);
    ASSERT_TRUE(isMagstimRemoved);

    // Give some time to the message to arrive
    logger.giveTimeToUpdate();
    ASSERT_TRUE(
        logger.contains("The device DelsysEmgDevice is now disconnected"));
    ASSERT_TRUE(
        logger.contains("The device DelsysAnalogDevice is now disconnected"));
    ASSERT_TRUE(
        logger.contains("The device MagstimRapidDevice is now disconnected"));
  }
  ASSERT_TRUE(logger.contains("Server has shut down"));
  logger.clear();
}

TEST(Server, Recording) {
  auto logger = TestLogger();

  // Happy path
  {
    server::TcpServerMock server(5000, 5001, 5002, 5003,
                                 sufficientTimeoutPeriod);
    server.startServer();

    server::TcpClient client;
    client.connect();

    // Add the devices
    client.addDelsysEmgDevice();
    client.addMagstimDevice();

    // Start recording
    bool isRecordingStarted = client.startRecording();
    ASSERT_TRUE(isRecordingStarted);

    // Give some time to the message to arrive
    logger.giveTimeToUpdate();
    ASSERT_TRUE(logger.contains(
        "The data collector DelsysEmgDataCollector is now recording"));
    logger.clear();
  }
  ASSERT_TRUE(logger.contains(
      "The data collector DelsysEmgDataCollector has stopped recording"));
  ASSERT_TRUE(logger.contains("Server has shut down"));
  logger.clear();

  // Start/Stop recording twice
  {
    server::TcpServerMock server(5000, 5001, 5002, 5003,
                                 sufficientTimeoutPeriod);
    server.startServer();

    server::TcpClient client;
    client.connect();

    // Add the devices
    bool isDelsysAdded = client.addDelsysEmgDevice();
    bool isMagstimAdded = client.addMagstimDevice();
    ASSERT_TRUE(isDelsysAdded);
    ASSERT_TRUE(isMagstimAdded);

    // Start recording
    bool isRecordingStarted = client.startRecording();
    ASSERT_TRUE(isRecordingStarted);

    // Start recording again
    isRecordingStarted = client.startRecording();
    ASSERT_TRUE(isRecordingStarted);

    // Give some time to the message to arrive
    logger.giveTimeToUpdate();
    ASSERT_TRUE(logger.contains(
        "The data collector DelsysEmgDataCollector is already recording"));
    logger.clear();

    // Stop recording
    bool isRecordingStopped = client.stopRecording();
    ASSERT_TRUE(isRecordingStopped);

    // Stop recording again
    isRecordingStopped = client.stopRecording();
    ASSERT_TRUE(isRecordingStopped);

    // Give some time to the message to arrive
    logger.giveTimeToUpdate();
    ASSERT_TRUE(logger.contains(
        "The data collector DelsysEmgDataCollector is not recording"));
  }
  logger.clear();
}

TEST(Server, LastTrialData) {
  auto logger = TestLogger();

  server::TcpServerMock server(5000, 5001, 5002, 5003, sufficientTimeoutPeriod);
  server.startServer();

  server::TcpClient client;
  client.connect();

  // Add the devices
  client.addDelsysEmgDevice();
  client.addMagstimDevice();

  client.startRecording();
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  client.stopRecording();

  // Get the data
  auto data = client.getLastTrialData();
  ASSERT_GE(data["DelsysEmgDataCollector"].size(), 900); // Should be ~1000
}

TEST(Server, addAnalyzer) {
  auto logger = TestLogger();

  server::TcpServerMock server(5000, 5001, 5002, 5003, sufficientTimeoutPeriod);
  server.startServer();

  server::TcpClient client;
  client.connect();

  // Add an analysis on using the DelsysEmgDataCollector
  bool isAnalyzerAdded = client.addAnalyzer(nlohmann::json::parse(R"({
    "name" : "Left Foot",
    "analyzer_type" : "cyclic_timed_events",
    "time_reference_device" : "DelsysAnalogDataCollector",
    "learning_rate" : 0.5,
    "initial_phase_durations" : [400, 600],
    "events" : [
      {
        "name" : "heel_strike",
        "previous" : "toe_off",
        "start_when" : [
          {
            "type": "threshold",
            "device" : "DelsysAnalogDataCollector",
            "channel" : 0,
            "comparator" : ">=",
            "value" : 0.2
          }
        ]
      },
      {
        "name" : "toe_off",
        "previous" : "heel_strike",
        "start_when" : [
          {
            "type": "threshold",
            "device" : "DelsysAnalogDataCollector",
            "channel" : 0,
            "comparator" : "<=",
            "value" : -0.2
          }
        ]
      }
    ]
  })"));
  ASSERT_TRUE(isAnalyzerAdded);

  // Give some time to the message to arrive
  logger.giveTimeToUpdate();
  ASSERT_TRUE(logger.contains(
      "Creating a cyclic timed events analyzer (Left Foot) from analogs"));

  // There is no way to collect the predicted data in the current client
  // implementation. So simply shut things out

  size_t idAnalyzer = server.getAnalyzers().getAnalyzerId("Left Foot");
  bool isAnalyzerRemoved = client.removeAnalyzer("Left Foot");
  ASSERT_TRUE(isAnalyzerRemoved);
  logger.giveTimeToUpdate();
  ASSERT_TRUE(logger.contains("Removing analyzer with id " +
                              std::to_string(idAnalyzer) + " (Left Foot)"));

  // Add a badly formatted analysis
  bool isBadAnalyzerAdded =
      client.addAnalyzer(nlohmann::json::parse(R"({"name" : "Invalid"})"));
  ASSERT_FALSE(isBadAnalyzerAdded);
  isBadAnalyzerAdded = client.addAnalyzer(nlohmann::json::parse(
      R"({"name" : "Invalid", "analyzer_type": "cyclic_timed_events"})"));
  ASSERT_FALSE(isBadAnalyzerAdded);
  isBadAnalyzerAdded = client.addAnalyzer(nlohmann::json::parse(
      R"({
        "name" : "Invalid", 
        "analyzer_type": "cyclic_timed_events",
        "time_reference_device" : "Nope",
        "learning_rate" : 0.5,
        "initial_phase_durations" : [400, 600]
      })"));
  ASSERT_FALSE(isBadAnalyzerAdded);
  isBadAnalyzerAdded = client.addAnalyzer(nlohmann::json::parse(
      R"({
      "name" : "Invalid", 
      "analyzer_type": "cyclic_timed_events",
      "time_reference_device" : "Nope",
      "learning_rate" : 0.5,
      "initial_phase_durations" : [400, 600],
      "events": [{
        "name" : "heel_strike",
        "previous" : "toe_off",
        "start_when" : [
          {
            "type": "threshold",
            "device" : "DelsysAnalogDataCollector",
            "comparator" : ">=",
            "value" : 0.2
          }
        ]
      }]
    })"));
  ASSERT_FALSE(isBadAnalyzerAdded);

  isBadAnalyzerAdded = client.addAnalyzer(nlohmann::json::parse(
      R"({
        "name" : "Invalid", 
        "analyzer_type": "cyclic_timed_events",
        "time_reference_device" : "Nope",
        "learning_rate" : 0.5,
        "initial_phase_durations" : [400, 600],
        "events": [{
          "name" : "heel_strike",
          "previous" : "toe_off",
          "start_when" : [
            {
              "type": "direction"
            }
          ]
        }]
      })"));
  ASSERT_FALSE(isBadAnalyzerAdded);

  // Remove an analysis that does not exist
  bool isWrongAnalyzerRemoved = client.removeAnalyzer("Invalid");
  ASSERT_FALSE(isWrongAnalyzerRemoved);
}
#endif // SKIP_CI_FAILING_TESTS
