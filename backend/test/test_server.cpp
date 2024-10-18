#include <gtest/gtest.h>
#include <iostream>
#include <thread>

#include "Server/TcpClient.h"
#include "Server/TcpServer.h"
#include "utils.h"

static double requiredPrecision(1e-10);

using namespace STIMWALKER_NAMESPACE;

TEST(Server, Ports) {
  auto logger = TestLogger();

  {
    server::TcpServer server;
    ASSERT_EQ(server.getCommandPort(), 5000);
    ASSERT_EQ(server.getDataPort(), 5001);
  }

  {
    server::TcpServer server(5010, 5011);
    ASSERT_EQ(server.getCommandPort(), 5010);
    ASSERT_EQ(server.getDataPort(), 5011);
  }
}

TEST(Server, StartServer) {
  auto logger = TestLogger();

  server::TcpServer server;
  server.startServer();
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  // Test the server actually started
  ASSERT_TRUE(logger.contains("Command server started on port 5000"));
  ASSERT_TRUE(logger.contains("Data server started on port 5001"));
  logger.clear();

  // Stop the server
  server.stopServer();

  // Test the server actually stopped
  ASSERT_TRUE(logger.contains(
      "Stopping listening to ports as server is shutting down"));
  ASSERT_TRUE(logger.contains("Server has shut down"));
}

TEST(Server, ClientConnexion) {
  auto logger = TestLogger();

  {
    server::TcpServerMock server(5000, 5001, std::chrono::milliseconds(500));
    server.startServer();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    ASSERT_TRUE(logger.contains("Command server started on port 5000"));
    ASSERT_TRUE(logger.contains("Data server started on port 5001"));
    ASSERT_TRUE(logger.contains("Waiting for a new connexion"));

    // Server can be shut if nothing happened
    server.stopServer();
    ASSERT_TRUE(logger.contains("Server has shut down"));
  }
  logger.clear();

  {
    server::TcpServerMock server(5000, 5001, std::chrono::milliseconds(500));
    server.startServer();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    asio::io_context context;
    asio::ip::tcp::resolver resolver(context);
    auto socket = std::make_unique<asio::ip::tcp::socket>(context);
    asio::connect(*socket, resolver.resolve("localhost", std::to_string(5000)));
    // Give some time to the message to arrive
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ASSERT_TRUE(logger.contains("Command socket connected to client, waiting "
                                "for a connexion to the data socket"));
  }
  // Server can be shut if one connexion was made and is waiting for the data
  ASSERT_TRUE(logger.contains("Server has shut down"));
  logger.clear();

  // Server can be shut if one connexion was made but dropped
  {
    server::TcpServerMock server(5000, 5001, std::chrono::milliseconds(500));
    server.startServer();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    asio::io_context context;
    asio::ip::tcp::resolver resolver(context);
    auto socket = std::make_unique<asio::ip::tcp::socket>(context);
    asio::connect(*socket, resolver.resolve("localhost", std::to_string(5000)));

    // Wait longer than the timeout
    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    ASSERT_TRUE(logger.contains(
        "Data socket connection timeout (500 ms), disconnecting client"));
  }
  ASSERT_TRUE("All devices are now disconnected");
  ASSERT_TRUE("Stopping listening to ports as server is shutting down");
  ASSERT_TRUE(logger.contains("Server has shut down"));
  logger.clear();

  // Server can be shut if connexions to command and data were made, but waiting
  // for the handshake
  {
    server::TcpServerMock server(5000, 5001, std::chrono::milliseconds(500));
    server.startServer();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    asio::io_context context;
    asio::ip::tcp::resolver resolver(context);
    auto commandSocket = std::make_unique<asio::ip::tcp::socket>(context);
    auto dataSocket = std::make_unique<asio::ip::tcp::socket>(context);
    asio::connect(*commandSocket,
                  resolver.resolve("localhost", std::to_string(5000)));
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    asio::connect(*dataSocket,
                  resolver.resolve("localhost", std::to_string(5001)));

    // Give some time to the message to arrive
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ASSERT_TRUE(logger.contains(
        "Data socket connected to client, waiting for official handshake"));
  }
  ASSERT_TRUE(logger.contains("Disconnecting client"));
  ASSERT_TRUE(logger.contains("Server has shut down"));
  logger.clear();

  // Server can be shut if connexions to command and data were made, but waiting
  // for the handshake timed out
  {
    server::TcpServerMock server(5000, 5001, std::chrono::milliseconds(500));
    server.startServer();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    asio::io_context context;
    asio::ip::tcp::resolver resolver(context);
    auto commandSocket = std::make_unique<asio::ip::tcp::socket>(context);
    auto dataSocket = std::make_unique<asio::ip::tcp::socket>(context);
    asio::connect(*commandSocket,
                  resolver.resolve("localhost", std::to_string(5000)));
    asio::connect(*dataSocket,
                  resolver.resolve("localhost", std::to_string(5001)));

    // Wait longer than the timeout
    std::this_thread::sleep_for(std::chrono::milliseconds(600));

    ASSERT_TRUE(
        logger.contains("Handshake timeout (500 ms), disconnecting client"));
    ASSERT_TRUE(logger.contains("Disconnecting client"));
    ASSERT_TRUE(logger.contains("All devices are now disconnected"));
  }
  ASSERT_TRUE(logger.contains("Server has shut down"));
  logger.clear();

  // Server can be shut if connexions to command and data were made, but waiting
  // for the handshake timed out, and a connexion was retried
  {
    server::TcpServerMock server(5000, 5001, std::chrono::milliseconds(500));
    server.startServer();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    asio::io_context context;
    asio::ip::tcp::resolver resolver(context);
    auto commandSocket = std::make_unique<asio::ip::tcp::socket>(context);
    auto dataSocket = std::make_unique<asio::ip::tcp::socket>(context);
    asio::connect(*commandSocket,
                  resolver.resolve("localhost", std::to_string(5000)));
    asio::connect(*dataSocket,
                  resolver.resolve("localhost", std::to_string(5001)));

    // Wait longer than the timeout
    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    asio::connect(*commandSocket,
                  resolver.resolve("localhost", std::to_string(5000)));

    // Give some time to the message to arrive
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ASSERT_EQ(logger.count("Command socket connected to client, waiting for a "
                           "connexion to the data socket"),
              2);
  }
  ASSERT_TRUE(logger.contains("Server has shut down"));
  logger.clear();

  // Server can be shut if connexions to command and data were made, but waiting
  // for the handshake timed out, and a connexion was retried but data connexion
  // timed out
  {
    server::TcpServerMock server(5000, 5001, std::chrono::milliseconds(500));
    server.startServer();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    asio::io_context context;
    asio::ip::tcp::resolver resolver(context);
    auto commandSocket = std::make_unique<asio::ip::tcp::socket>(context);
    auto dataSocket = std::make_unique<asio::ip::tcp::socket>(context);
    asio::connect(*commandSocket,
                  resolver.resolve("localhost", std::to_string(5000)));
    asio::connect(*dataSocket,
                  resolver.resolve("localhost", std::to_string(5001)));

    // Wait longer than the timeout
    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    asio::connect(*commandSocket,
                  resolver.resolve("localhost", std::to_string(5000)));
    std::this_thread::sleep_for(std::chrono::milliseconds(600));

    ASSERT_TRUE(logger.contains("Data socket connection timeout (500 ms), "
                                "disconnecting client"));
  }
  ASSERT_TRUE(logger.contains("Server has shut down"));
  logger.clear();

  // Server can be shut if connexions to command and data were made, but waiting
  // for the handshake timed out, and a connexion was retried and data connexion
  // was made
  {
    server::TcpServerMock server(5000, 5001, std::chrono::milliseconds(500));
    server.startServer();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    asio::io_context context;
    asio::ip::tcp::resolver resolver(context);
    auto commandSocket = std::make_unique<asio::ip::tcp::socket>(context);
    auto dataSocket = std::make_unique<asio::ip::tcp::socket>(context);
    asio::connect(*commandSocket,
                  resolver.resolve("localhost", std::to_string(5000)));
    asio::connect(*dataSocket,
                  resolver.resolve("localhost", std::to_string(5001)));

    // Wait longer than the timeout
    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    asio::connect(*commandSocket,
                  resolver.resolve("localhost", std::to_string(5000)));
    asio::connect(*dataSocket,
                  resolver.resolve("localhost", std::to_string(5001)));

    // Give some time to the message to arrive
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ASSERT_EQ(logger.count("Data socket connected to client, waiting for "
                           "official handshake"),
              2);
  }
  ASSERT_TRUE(logger.contains("Server has shut down"));
  logger.clear();

  // Server can be shut if connexions to command and data were made, but waiting
  // for the handshake timed out, and a connexion was retried and data connexion
  // was made, but handshaked timed out
  {
    server::TcpServerMock server(5000, 5001, std::chrono::milliseconds(500));
    server.startServer();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    asio::io_context context;
    asio::ip::tcp::resolver resolver(context);
    auto commandSocket = std::make_unique<asio::ip::tcp::socket>(context);
    auto dataSocket = std::make_unique<asio::ip::tcp::socket>(context);
    asio::connect(*commandSocket,
                  resolver.resolve("localhost", std::to_string(5000)));
    asio::connect(*dataSocket,
                  resolver.resolve("localhost", std::to_string(5001)));

    // Wait longer than the timeout
    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    asio::connect(*commandSocket,
                  resolver.resolve("localhost", std::to_string(5000)));
    asio::connect(*dataSocket,
                  resolver.resolve("localhost", std::to_string(5001)));
    std::this_thread::sleep_for(std::chrono::milliseconds(600));

    ASSERT_EQ(logger.count("Handshake timeout (500 ms), disconnecting client"),
              2);
  }
  ASSERT_TRUE(logger.contains("Server has shut down"));
  logger.clear();

  // Server can be shut if connexions was successful
  {
    server::TcpServerMock server(5000, 5001, std::chrono::milliseconds(500));
    server.startServer();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    server::TcpClient client;
    client.connect();

    // Give some time to the message to arrive
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ASSERT_TRUE(logger.contains("Handshake from client is valid"));
  }
  ASSERT_TRUE(logger.contains("Server has shut down"));
  logger.clear();

  // Server can be shut if connexions was successful then dropped
  {
    server::TcpServerMock server(5000, 5001, std::chrono::milliseconds(500));
    server.startServer();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    server::TcpClient client;
    client.connect();
    client.disconnect();

    // Give some time to the message to arrive
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ASSERT_TRUE(logger.contains("Disconnecting client"));
  }
  ASSERT_TRUE(logger.contains("Server has shut down"));
  logger.clear();

  // We can reconnect after disconnecting
  {
    server::TcpServerMock server(5000, 5001, std::chrono::milliseconds(500));
    server.startServer();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    server::TcpClient client;
    client.connect();
    client.disconnect();

    // Give some time
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    client.connect();

    ASSERT_EQ(logger.count("Handshake from client is valid"), 2);
  }
  ASSERT_TRUE(logger.contains("Server has shut down"));
  logger.clear();
}
